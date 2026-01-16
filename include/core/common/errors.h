#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    ERROR_NONE = 0,
    ERROR_NULL_POINTER,
    ERROR_INVALID_ARGUMENT,
    ERROR_MEMORY_ALLOCATION,
    ERROR_HARDWARE_FAILURE,
    ERROR_TIMEOUT,
    ERROR_BUFFER_OVERFLOW,
    ERROR_NOT_INITIALIZED,
    ERROR_ALREADY_INITIALIZED,
    ERROR_NOT_IMPLEMENTED,
    ERROR_DRIVER_BUSY,
    ERROR_CONTROLLER_BUSY,
    ERROR_STORAGE_FULL,
    ERROR_STORAGE_IO,
    ERROR_CALIBRATION_FAILED,
    ERROR_COMMUNICATION,
    ERROR_UNEXPECTED_STATE,
    
    // Add more specific errors as needed
    ERROR_TOTAL_COUNT  // Keep this last
} error_code_t;

// Function to get error string
const char* error_to_string(error_code_t error);

#endif // ERRORS_H