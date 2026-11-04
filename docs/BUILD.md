# Build und Tests

Die SDK-Quellen liegen unter `vendor/hollyhock-3` und sind auf Commit
`1e483e395391d50d9bd2d21cee4beecc7589fb1c` festgesetzt. Ein rekursiver
Submodule-Checkout ist nicht erforderlich.

## HH3 bauen

Benötigt werden Docker und Python 3. Das Build-Image enthält GCC 15.2.0 und
ist auf folgenden Digest festgelegt:

```text
ghcr.io/classpaddev/hollyhock-3@sha256:4381a460f8b19411552ac30f2aba73da38d3aba535412a66c69f8528fe94b49a
```

Unter Linux:

```sh
sh tools/build.sh
```

Unter Windows mit einer WSL-Distribution, in der Docker verfügbar ist:

```powershell
.\build.ps1
# Eine bestimmte WSL-Distribution verwenden:
.\build.ps1 -Distribution Ubuntu
```

Das Compiler-Image wird bei Bedarf von Docker heruntergeladen. Während des
Builds ist der Container ohne Netzwerkzugriff und hat Zugriff auf den
Projektordner. Das Ergebnis ist `dist/mathe.hh3`.

Mit einer lokal eingerichteten `sh4a_nofpueb-elf-`-Toolchain:

```sh
make -j2 hh3
make verify
```

## Hosttests

Unter Linux mit g++, C++20-Unterstützung, Python 3 und Sanitizern:

```sh
make test
```

Die Tests prüfen Rechenfunktionen, numerische Sonderfälle, Eingabehilfen,
Touch- und Tastennavigation, Bildschirmsicherung und die Speichergrenzen.
AddressSanitizer, UndefinedBehaviorSanitizer und LeakSanitizer sind aktiviert.
Gerenderte Bildpuffer entstehen unter `docs/preview`.

Optional PNG-Vorschauen erzeugen (Pillow erforderlich):

```sh
python3 tools/preview.py
```

## Pakete erzeugen

Nach erfolgreichen Host- und Binaryprüfungen für den aktuellen Quellstand:

```sh
python3 tools/package.py
```

Unter `dist` entstehen die einzelne HH3-Datei, ein App-ZIP mit
Installationsanleitung und Lizenzen, ein vollständiges Quellpaket und
`SHA256SUMS.txt`. Generierte Prüfprotokolle bleiben im lokalen `dist`-Ordner.

## Speicherlayout

Das Programm wird in den VBAK-Bereich `[0x8C052800, 0x8C0A5000)` gelinkt.
Eigene `calcInit`-/`calcExit`-Hooks sichern den Bildschirm auf dem OS-Heap
und stellen ihn beim Ende wieder her. Die Binaryprüfung kontrolliert
Ladesegmente samt BSS, Einstiegspunkt, Hook-Definitionen und deren
Einbindung in die Initialisierungs- und Finalisierungsarrays.
