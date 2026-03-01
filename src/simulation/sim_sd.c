// sd_sim_wrapper.c
#include "simulation/sim_sd.h"
#include <string.h>
#include <stdlib.h>

static File current_file;

// File method implementations
size_t file_print(File* file, const char* str) {
    if (!file || !file->is_open || !file->handle) return 0;
    return fprintf(file->handle, "%s", str);
}

size_t file_print_int(File* file, int value) {
    if (!file || !file->is_open || !file->handle) return 0;
    return fprintf(file->handle, "%d", value);
}

size_t file_print_float(File* file, double value) {
    if (!file || !file->is_open || !file->handle) return 0;
    return fprintf(file->handle, "%g", value);
}

size_t file_println(File* file) {
    if (!file || !file->is_open || !file->handle) return 0;
    return fprintf(file->handle, "\n");
}

size_t file_println_int(File* file, int value) {
    size_t len = file_print_int(file, value);
    len += file_println(file);
    return len;
}

size_t file_println_float(File* file, double value) {
    size_t len = file_print_float(file, value);
    len += file_println(file);
    return len;
}

size_t file_println_str(File* file, const char* str) {
    size_t len = file_print(file, str);
    len += file_println(file);
    return len;
}

int file_available(File* file) {
    if (!file || !file->is_open || !file->handle) return 0;
    long current = ftell(file->handle);
    fseek(file->handle, 0, SEEK_END);
    long end = ftell(file->handle);
    fseek(file->handle, current, SEEK_SET);
    return (end - current) > 0 ? 1 : 0;
}

int file_read(File* file) {
    if (!file || !file->is_open || !file->handle) return -1;
    return fgetc(file->handle);
}

int file_peek(File* file) {
    if (!file || !file->is_open || !file->handle) return -1;
    int c = fgetc(file->handle);
    if (c != EOF) {
        ungetc(c, file->handle);
    }
    return c;
}

void file_flush(File* file) {
    if (file && file->is_open && file->handle) {
        fflush(file->handle);
    }
}

int file_seek(File* file, uint32_t pos) {
    if (!file || !file->is_open || !file->handle) return 0;
    return fseek(file->handle, pos, SEEK_SET) == 0;
}

uint32_t file_position(File* file) {
    if (!file || !file->is_open || !file->handle) return 0;
    return ftell(file->handle);
}

uint32_t file_size(File* file) {
    if (!file || !file->is_open || !file->handle) return 0;
    long current = ftell(file->handle);
    fseek(file->handle, 0, SEEK_END);
    long size = ftell(file->handle);
    fseek(file->handle, current, SEEK_SET);
    return size;
}

void file_close(File* file) {
    if (file && file->is_open && file->handle) {
        fclose(file->handle);
        file->handle = NULL;
        file->is_open = 0;
    }
}

int file_is_open(File* file) {
    return file && file->is_open;
}

// SD method implementations
static int sim_begin(uint8_t cs_pin) {
    create_folder(OUTPUT_FOLDER_NAME);
    create_folder(FLIGHT_PROFILES_FOLDER_NAME);
    return 1;
}

static int sim_begin_with_clock(uint8_t cs_pin, uint32_t clock) {
    // Clock speed ignored in simulation
    return sim_begin(cs_pin);
}

static File* sim_open(const char* filepath, int mode) {
    
    char fullpath[256];
    
    if (strstr(filepath, "/") == filepath) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", 
                 OUTPUT_FOLDER_NAME, filepath + 1);
    } else {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", 
                 OUTPUT_FOLDER_NAME, filepath);
    }
    
    // Determine fopen mode based on the mode parameter
    const char* fopen_mode;
    switch (mode) {
        case FILE_READ:
            fopen_mode = "r";
            break;
        case FILE_WRITE:
            fopen_mode = "a";  // Overwrites existing file
            break;
        default:
            printf("Unknown mode: %d, defaulting to 'r'\n", mode);
            fopen_mode = "r";
            break;
    }
    
    FILE* handle = fopen(fullpath, fopen_mode);
    if (!handle) return NULL;
    
    // Initialize the File object with all method pointers
    current_file.handle = handle;
    current_file.is_open = true;
    strncpy(current_file.name, filepath, sizeof(current_file.name) - 1);
    
    // Assign all method pointers (same as before)
    current_file.print = file_print;
    current_file.print_int = file_print_int;
    current_file.print_float = file_print_float;
    current_file.println = file_println;
    current_file.println_int = file_println_int;
    current_file.println_float = file_println_float;
    current_file.println_str = file_println_str;
    current_file.available = file_available;
    current_file.read = file_read;
    current_file.peek = file_peek;
    current_file.flush = file_flush;
    current_file.seek = file_seek;
    current_file.position = file_position;
    current_file.size = file_size;
    current_file.close = file_close;
    
    //seek to the end and return
    fseek(handle, 0, SEEK_END);
    return &current_file;
}

static int sim_exists(const char* filepath) {
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", 
             OUTPUT_FOLDER_NAME, 
             (strchr(filepath, '/') ? strchr(filepath, '/') + 1 : filepath));
    
    FILE* f = fopen(fullpath, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

static int sim_mkdir(const char* filepath) {
    create_folder((char*)filepath);
    return 1;
}

static int sim_remove(const char* filepath) {
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", 
             OUTPUT_FOLDER_NAME, 
             (strchr(filepath, '/') ? strchr(filepath, '/') + 1 : filepath));
    
    return remove(fullpath) == 0 ? 1 : 0;
}

static int sim_rmdir(const char* filepath) {
    // Simple implementation - just try to remove the directory
    return rmdir(filepath) == 0 ? 1 : 0;
}

static File* sim_openRoot(void) {
    // Not typically used - return NULL or a special handle
    return NULL;
}

// Global SD instance with all methods
const SDClass SD = {
    .begin = sim_begin,
    .begin_with_clock = sim_begin_with_clock,
    .open = sim_open,
    .exists = sim_exists,
    .mkdir = sim_mkdir,
    .remove = sim_remove,
    .rmdir = sim_rmdir,
    .openRoot = sim_openRoot
};