"""ClassPad HH3 ELF layout checks, independent of the host architecture."""
import struct

APP_START = 0x8C052800
APP_END = 0x8C0A5000


def require(condition, message):
    if not condition:
        raise ValueError(message)


def check_load_ranges(loads, entry):
    require(bool(loads), "No LOAD segments")
    occupied = []
    for segment in loads:
        start, size = segment["vaddr"], segment["memory_bytes"]
        end = start + size
        require(size > 0, "Empty LOAD segment")
        require(APP_START <= start < end <= APP_END,
                f"LOAD range outside VBAK: {start:#x}..{end:#x}; allowed {APP_START:#x}..{APP_END:#x}")
        require(start % 4 == 0, "Unaligned LOAD address")
        require(0 <= segment["file_bytes"] <= size, "Illegal LOAD sizes")
        require(segment["flags"] and not segment["flags"] & ~7, "Illegal LOAD flags")
        occupied.append((start, end))
    occupied.sort()
    for previous, current in zip(occupied, occupied[1:]):
        require(previous[1] <= current[0], "Overlapping LOAD segments")
    require(entry % 2 == 0, "Unaligned SH entry point")
    require(any(s["flags"] & 1 and s["vaddr"] <= entry < s["vaddr"] + s["file_bytes"] for s in loads),
            "Entry point is outside loaded executable bytes")


def inspect_elf(blob):
    require(blob[:7] == b"\x7fELF\x01\x02\x01", "Expected big-endian ELF32")
    elf = struct.unpack_from(">HHIIIIIHHHHHH", blob, 16)
    kind, machine, version, entry, phoff, shoff, flags, ehsize, phsize, phnum, shsize, shnum, shstr = elf
    require(kind == 2 and machine == 42 and flags == 0x11, "Expected executable SH4a-nofpu")
    require(ehsize == 52 and phsize == 32 and shsize == 40, "Unexpected ELF structure sizes")
    require(phoff + phnum * phsize <= len(blob), "Truncated program headers")
    loads, notes = [], {}
    for i in range(phnum):
        p = struct.unpack_from(">IIIIIIII", blob, phoff + i * phsize)
        ptype, offset, vaddr, paddr, filesz, memsz, pflags, align = p
        require(offset + filesz <= len(blob), "Truncated segment")
        if ptype == 1:
            require(vaddr == paddr, "Expected fixed physical load addresses")
            require(align in (0, 1) or (align & (align - 1) == 0 and offset % align == vaddr % align),
                    "Invalid segment alignment")
            loads.append({"vaddr": vaddr, "file_bytes": filesz, "memory_bytes": memsz, "flags": pflags})
        if ptype == 4:
            cursor, end = offset, offset + filesz
            while cursor < end:
                require(cursor + 12 <= end, "Truncated note header")
                namesz, descsz, ntype = struct.unpack_from(">III", blob, cursor)
                cursor += 12
                name_end = cursor + ((namesz + 3) & ~3)
                desc_end = name_end + ((descsz + 3) & ~3)
                require(desc_end <= end and ntype == 1, "Invalid HH3 note")
                name = blob[cursor:cursor + namesz].rstrip(b"\0").decode("ascii")
                notes[name] = blob[name_end:name_end + descsz].rstrip(b"\0").decode("ascii")
                cursor = desc_end
    check_load_ranges(loads, entry)
    require(shoff + shnum * shsize <= len(blob) and shstr < shnum, "Invalid section headers")
    sections = [struct.unpack_from(">IIIIIIIIII", blob, shoff + i * shsize) for i in range(shnum)]
    strings = sections[shstr]
    names = blob[strings[4]:strings[4] + strings[5]]
    named = {}
    for section in sections:
        name = names[section[0]:].split(b"\0", 1)[0].decode("ascii")
        require(section[1] == 8 or section[4] + section[5] <= len(blob), "Truncated section")
        require(section[1] not in (4, 9) or section[5] == 0, "Unapplied ELF relocations")
        named[name] = section
    require(".init_array" in named and ".fini_array" in named and ".bss" in named, "Missing C++ startup sections")
    return {"entry": entry, "loads": loads, "notes": notes, "sections": sections, "named": named}


def startup_hooks(unstripped, image):
    """Require strong overrides and actual calls from the initializer arrays."""
    full = inspect_elf(unstripped)
    require(full["entry"] == image["entry"] and full["loads"] == image["loads"], "ELF/HH3 load images differ")
    symbols = {}
    for section in full["sections"]:
        if section[1] != 2:
            continue
        strings = full["sections"][section[6]]
        names = unstripped[strings[4]:strings[4] + strings[5]]
        require(section[9] == 16, "Unexpected symbol size")
        for offset in range(section[4], section[4] + section[5], 16):
            name, value, size, info, other, shndx = struct.unpack_from(">IIIBBH", unstripped, offset)
            label = names[name:].split(b"\0", 1)[0].decode("ascii")
            symbols[label] = (value, size, info, shndx)
    hooks = {}
    for name in ("_calcInit", "_calcExit", "_relocate_sdk"):
        require(name in symbols, f"Missing startup hook {name}")
        value, size, info, shndx = symbols[name]
        require(info >> 4 == 1 and info & 15 == 2 and shndx != 0 and size > 0,
                f"Expected a strong function definition: {name}")
        hooks[name] = value
    arrays = {}
    for name in (".init_array", ".fini_array"):
        s = full["named"][name]
        require(s[5] % 4 == 0, "Unaligned initializer array")
        arrays[name] = list(struct.unpack_from(">" + "I" * (s[5] // 4), unstripped, s[4]))
    require(hooks["_calcInit"] in arrays[".init_array"], "Screen backup is not initialized")
    require(hooks["_calcExit"] in arrays[".fini_array"], "Screen backup is not restored")
    require(hooks["_relocate_sdk"] in arrays[".init_array"], "SDK relocation is not initialized")
    require(arrays[".init_array"].index(hooks["_relocate_sdk"]) < arrays[".init_array"].index(hooks["_calcInit"]),
            "Screen initialization runs before SDK relocation")
    return {name: hex(address) for name, address in hooks.items()}
