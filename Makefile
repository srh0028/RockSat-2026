# ============ COMPILER CONFIGURATION ============
# WSL -> Windows cross-compilation
CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O0 -Iinclude

# ============ DIRECTORY STRUCTURE ============
SRC_DIR = src
BIN_DIR = bin
TARGET = main.exe  # Windows executable

# Find all .c files in src directory
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Object files go in bin directory
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# ============ BUILD RULES ============
# Default target - builds Windows .exe
all: $(TARGET)

# Link object files into Windows executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@

# Compile C files to object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ============ UTILITY TARGETS ============
clean:
	rm -rf $(BIN_DIR) $(TARGET)

# Copy the .exe to Windows desktop (adjust path)
copy:
	cp $(TARGET) /mnt/c/Users/$(USER)/Desktop/

# Build and copy
release: all copy

# Test with Wine (Windows compatibility layer)
test: all
	@echo "Testing with Wine..."
	wine $(TARGET) 2>/dev/null || echo "Install Wine: sudo apt install wine"

# ============ PHONY TARGETS ============
.PHONY: all clean copy release test