# ============ COMPILER CONFIGURATION ============
CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O0 -Iinclude -Isrc/mount

# ============ PICO CONFIGURATION ============
PICO_SDK_PATH ?= $(HOME)/pico-sdk
PICO_COMPILER = arm-none-eabi-gcc
PICO_CFLAGS = -Wall -Wextra -std=c99 -O2 -g -mcpu=cortex-m0plus -mthumb \
              -I. -Iinclude -Isrc -Isrc/mount -DPICO_BUILD=1

# ============ DIRECTORY STRUCTURE ============
SRC_DIR = src
BIN_DIR = bin
TARGET = main.exe

# ============ PICO SOURCE FILES ============
# Core Pico files you want
PICO_CORE = \
	src/flight_software/controllers/testbed_temperature_controller.c \
	src/flight_software/drivers/testbed_temperature_driver.c

# Dependencies (from grep output)
PICO_DEPS = \
	src/flight_software/flight_software_types.c \
	src/simulation/generic_mount.c

# Add any other dependencies these files might need
# (check if these include other local files)
PICO_EXTRA_DEPS = \
	src/flight_software/controllers/testbed_temperature_controller.c \
	src/flight_software/drivers/testbed_temperature_driver.c

# Combine all needed Pico files
PICO_SRCS = $(PICO_CORE) $(PICO_DEPS)
PICO_OBJS = $(PICO_SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/pico_%.o)

# ============ SIMULATOR BUILD (DEFAULT) ============
# Recursively find all .c files for simulator
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ============ PICO BUILD (make pico) ============
pico: check_pico_sdk check_pico_tools $(BIN_DIR)/pico_firmware.uf2

# Check if Pico SDK exists
check_pico_sdk:
	@if [ ! -d "$(PICO_SDK_PATH)" ]; then \
		echo "Error: PICO_SDK_PATH not set or invalid"; \
		echo "Set it with: export PICO_SDK_PATH=/path/to/pico-sdk"; \
		exit 1; \
	fi

# Check if elf2uf2 is installed
check_pico_tools:
	@which elf2uf2 > /dev/null || (echo "Error: elf2uf2 not found. Install with: sudo apt install elf2uf2"; exit 1)
	@which arm-none-eabi-gcc > /dev/null || (echo "Error: arm-none-eabi-gcc not found. Install with: sudo apt install gcc-arm-none-eabi"; exit 1)

# Build Pico ELF file
$(BIN_DIR)/pico_firmware.elf: $(PICO_OBJS) | $(BIN_DIR)
	$(PICO_COMPILER) $(PICO_CFLAGS) $(PICO_OBJS) \
		-T $(PICO_SDK_PATH)/src/rp2_common/pico_standard_link/memmap_default.ld \
		-L $(PICO_SDK_PATH)/src/rp2040/hardware_regs \
		-L $(PICO_SDK_PATH)/src/rp2_common/pico_stdlib \
		-Wl,-Map,$(BIN_DIR)/pico_firmware.map \
		--specs=nosys.specs \
		-Wl,--wrap,sprintf \
		-o $@ \
		-lpico_stdlib -lpico_hal -lpico_printf

# Convert ELF to UF2
$(BIN_DIR)/pico_firmware.uf2: $(BIN_DIR)/pico_firmware.elf
	elf2uf2 $< $@

# Compile Pico object files
$(BIN_DIR)/pico_%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(PICO_COMPILER) $(PICO_CFLAGS) -c $< -o $@

# ============ COMMON RULES ============
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(TARGET)

clean_pico:
	rm -rf $(BIN_DIR)/pico_*.o $(BIN_DIR)/pico_firmware.*

copy:
	cp $(TARGET) /mnt/c/Users/$(USER)/Desktop/

release: all copy

test: all
	@wine $(TARGET) 2>/dev/null || echo "Install Wine: sudo apt install wine"

# Show what files will be included in Pico build
pico_files:
	@echo "Pico source files:"
	@for f in $(PICO_SRCS); do echo "  $$f"; done

.PHONY: all clean copy release test pico clean_pico check_pico_sdk check_pico_tools pico_files