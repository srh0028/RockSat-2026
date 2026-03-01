// sd_sim_wrapper.h
#ifndef SD_SIM_WRAPPER_H
#define SD_SIM_WRAPPER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "storage.h"

#define FILE_READ 0
#define FILE_WRITE 1

// This simulates the Arduino File class
typedef struct File {
    FILE* handle;
    char name[64];
    bool is_open;
    
    // Method pointers that mimic Arduino's File methods
    size_t (*print)(struct File* file, const char* str);
    size_t (*print_int)(struct File* file, int value);
    size_t (*print_float)(struct File* file, double value);
    size_t (*println)(struct File* file);
    size_t (*println_int)(struct File* file, int value);
    size_t (*println_float)(struct File* file, double value);
    size_t (*println_str)(struct File* file, const char* str);
    int (*available)(struct File* file);
    int (*read)(struct File* file);
    int (*peek)(struct File* file);
    void (*flush)(struct File* file);
    int (*seek)(struct File* file, uint32_t pos);
    uint32_t (*position)(struct File* file);
    uint32_t (*size)(struct File* file);
    void (*close)(struct File* file);
} File;

// This simulates the Arduino SD class
typedef struct {
    int (*begin)(uint8_t cs_pin);
    int (*begin_with_clock)(uint8_t cs_pin, uint32_t clock);
    File* (*open)(const char* filepath, int mode);
    int (*exists)(const char* filepath);
    int (*mkdir)(const char* filepath);
    int (*remove)(const char* filepath);
    int (*rmdir)(const char* filepath);
    File* (*openRoot)(void);  // Returns root directory File object
} SDClass;

// Global instance (just like Arduino's "SD")
extern const SDClass SD;

// Function declarations for File methods
size_t file_print(File* file, const char* str);
size_t file_print_int(File* file, int value);
size_t file_print_float(File* file, double value);
size_t file_println(File* file);
size_t file_println_int(File* file, int value);
size_t file_println_float(File* file, double value);
size_t file_println_str(File* file, const char* str);
int file_available(File* file);
int file_read(File* file);
int file_peek(File* file);
void file_flush(File* file);
int file_seek(File* file, uint32_t pos);
uint32_t file_position(File* file);
uint32_t file_size(File* file);
void file_close(File* file);

#endif