SDK_DIR = vendor/hollyhock-3/sdk
CROSS ?= sh4a_nofpueb-elf-
TARGET_CXX = $(CROSS)g++
HOST_CXX ?= g++
CORE = src/math.cpp src/catalog.cpp src/calculate.cpp src/screen_backup.cpp
UI = src/app.cpp src/input_groups.cpp src/input_help.cpp
WARN = -Wall -Wextra -Wpedantic
TARGET_FLAGS = -std=c++20 -Os -g -flto -ffat-lto-objects -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti $(WARN) -Werror -Isrc -I$(SDK_DIR)/include
# The default 0x8CFF0000 leaves only 64 KiB. Use the documented 330 KiB
# VBAK area; startup.cpp replaces the SDK's screen-backup hooks accordingly.
APP_BASE = 0x8C052800
OBJECTS = $(patsubst src/%.cpp,build/target/%.o,$(CORE) $(UI) src/startup.cpp src/main.cpp)

.PHONY: all hh3 verify test preview sdk
all: hh3
hh3: dist/mathe.hh3
verify: dist/mathe.hh3
	python3 -m unittest discover -s tests -p 'test_binary.py'
	python3 tools/check_binary.py
sdk:
	$(MAKE) -C $(SDK_DIR) -j2

$(SDK_DIR)/libsdk.a:
	$(MAKE) -C $(SDK_DIR) -j2

build/target/%.o: src/%.cpp src/math.hpp src/catalog.hpp src/app.hpp src/font.hpp src/screen_backup.hpp src/input_groups.hpp src/input_help.hpp Makefile
	@mkdir -p $(@D)
	$(TARGET_CXX) $(TARGET_FLAGS) -MMD -MP -c $< -o $@

dist/mathe.elf: $(OBJECTS) $(SDK_DIR)/libsdk.a
	@mkdir -p $(@D)
	$(TARGET_CXX) $(TARGET_FLAGS) -Wl,-Ttext-segment,$(APP_BASE),--gc-sections,-Map,dist/mathe.map -o $@ $(OBJECTS) -L$(SDK_DIR) -lsdk -lm

dist/mathe.hh3: dist/mathe.elf
	$(CROSS)strip -o $@ $<
	$(CROSS)size $@
	$(CROSS)readelf -h $@

build/tests: $(CORE) $(UI) tests/test.cpp src/math.hpp src/catalog.hpp src/app.hpp src/font.hpp src/screen_backup.hpp src/input_groups.hpp src/input_help.hpp Makefile
	@mkdir -p $(@D)
	$(HOST_CXX) -std=c++20 -O1 -g $(WARN) -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Isrc $(CORE) $(UI) tests/test.cpp -o $@

test: build/tests
	@mkdir -p docs/preview
	ASAN_OPTIONS=detect_leaks=1 python3 tools/test.py

-include $(OBJECTS:.o=.d)
