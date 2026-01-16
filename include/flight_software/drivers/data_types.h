/* AI GENERATED */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Common flag definitions for all data types
#define DATA_FLAG_NONE 0x0000
#define DATA_FLAG_CALIBRATED 0x0001
#define DATA_FLAG_SUSPECT 0x0002
#define DATA_FLAG_OVERFLOW 0x0004
#define DATA_FLAG_UNDERFLOW 0x0008
#define DATA_FLAG_TIMING_ERROR 0x0010
#define DATA_FLAG_HARDWARE_ERROR 0x0020
#define DATA_FLAG_CORRUPTED 0x0040
#define DATA_FLAG_TEST_DATA 0x0080

// Base data structure that all processed data should include
typedef struct
{
    uint32_t timestamp_ms;   // Sample timestamp
    uint16_t flags;          // Data quality flags
    uint8_t sequence_number; // Sample sequence number
    uint8_t reserved;        // Padding/alignment
} data_header_t;

// Example for Langmuir probe data
typedef struct
{
    data_header_t header;
    float bias_voltage;
    float measured_current;
    float temperature;
    float density;
} langmuir_processed_t;

// Example for EFI data
typedef struct
{
    data_header_t header;
    float e_field_x;
    float e_field_y;
    float e_field_z;
    float sun_angle;
    float photoemission_correction;
} efi_processed_t;

// Example for Sunlight sensor data
typedef struct
{
    data_header_t header;
    float sensor_values[4]; // Four sunlight sensors
    float spin_phase;
    float sun_vector[3];
} sunlight_processed_t;

#endif // DATA_TYPES_H