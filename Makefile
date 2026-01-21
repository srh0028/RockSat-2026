# ============ COMPILER CONFIGURATION ============
CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O0 -Iinclude -Isrc/mount

# ============ DIRECTORY STRUCTURE ============
SRC_DIR = src
BIN_DIR = bin
TARGET = main.exe

# Recursively find all .c files
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# ============ BUILD RULES ============
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(TARGET)

copy:
	cp $(TARGET) /mnt/c/Users/$(USER)/Desktop/

release: all copy

test: all
	@wine $(TARGET) 2>/dev/null || echo "Install Wine: sudo apt install wine"

.PHONY: all clean copy release test