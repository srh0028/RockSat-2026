# ============ COMPILER CONFIGURATION ============
# The compiler we're using - clang
CC = clang
# Compiler flags:
# -Wall     = Enable all common warnings
# -Wextra   = Enable extra warnings  
# -std=c99  = Use C99 standard
# -g        = Include debug symbols
# -O0       = No optimization (better for debugging)
# -Iinclude = Look for header files in the 'include' directory
CFLAGS = -Wall -Wextra -std=c99 -g -O0 -Iinclude
# Directory containing header files
INCLUDE_DIR = include

# ============ DIRECTORY STRUCTURE ============
# Directory containing our .c source files
SRC_DIR = src
# Directory where compiled files will go
BIN_DIR = bin
# Name of the final executable program
TARGET = main

# Find all .c files in src directory automatically
# This becomes: src/main.c (and any other .c files we add)
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Convert source file paths to object file paths
# src/main.c becomes bin/main.o
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# ============ BUILD RULES ============
# Default target - what runs when you type 'make'
all: $(BIN_DIR)/$(TARGET)

# Rule to link object files into final executable
# Depends on all object files being built first
# | $(BIN_DIR) means ensure bin directory exists first
$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@

# Rule to compile each .c file into a .o file
# % is a pattern that matches any file name
# $< is the source file (src/whatever.c)
# $@ is the target file (bin/whatever.o)
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ============ UTILITY TARGETS ============
# Remove all compiled files to clean up
clean:
	rm -rf $(BIN_DIR)

# Build and then run the program
run: $(BIN_DIR)/$(TARGET)
	./$(BIN_DIR)/$(TARGET)

# ============ PHONY TARGETS ============
# These are not actual files, just commands
.PHONY: all clean run