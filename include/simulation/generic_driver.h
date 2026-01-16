#ifndef GENERIC_DRIVER_H
#define GENERIC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "core/common/errors.h"

/**
 * @brief Generic driver template for instrument drivers
 *
 * This serves as a template for creating specific instrument drivers.
 * Modify for Langmuir, EFI, etc.
 */

// Forward declaration only
typedef struct generic_driver generic_driver_t;

typedef enum
{
    DRIVER_STATE_UNINITIALIZED = 0,
    DRIVER_STATE_IDLE,
    DRIVER_STATE_SAMPLING,
    DRIVER_STATE_CALIBRATING,
    DRIVER_STATE_ERROR,
    DRIVER_STATE_COUNT // Keep last for array sizing
} driver_state_t;

// Function pointer types for driver operations
typedef error_code_t (*driver_init_func_t)(generic_driver_t *driver, void *config);
typedef error_code_t (*driver_sample_func_t)(generic_driver_t *driver, void *sample_buffer);
typedef error_code_t (*driver_calibrate_func_t)(generic_driver_t *driver);
typedef error_code_t (*driver_emergency_func_t)(generic_driver_t *driver);
typedef error_code_t (*driver_cleanup_func_t)(generic_driver_t *driver);

typedef struct
{
    driver_init_func_t init;
    driver_sample_func_t sample;
    driver_calibrate_func_t calibrate;
    driver_emergency_func_t emergency;
    driver_cleanup_func_t cleanup;
    size_t sample_size; // Size of one sample in bytes
} driver_operations_t;

struct generic_driver
{
    uint8_t driver_id;
    char driver_name[32];
    driver_state_t state;
    const driver_operations_t *ops; // Instance-specific operations
    void *hardware_context;         // Hardware-specific context
    void *config;                   // Driver-specific configuration
    uint32_t sample_count;          // Total samples taken
    uint64_t last_sample_time;      // Timestamp of last sample
    bool is_operational;            // Emergency status flag
};

// Core driver API
error_code_t generic_driver_init(generic_driver_t *driver,
                                 const char *name,
                                 uint8_t id,
                                 const driver_operations_t *ops,
                                 void *config,
                                 void *hardware_context);

error_code_t generic_driver_take_sample(generic_driver_t *driver,
                                        void *sample_buffer);

error_code_t generic_driver_calibrate(generic_driver_t *driver);

error_code_t generic_driver_emergency_stop(generic_driver_t *driver);

error_code_t generic_driver_set_state(generic_driver_t *driver,
                                      driver_state_t new_state);

error_code_t generic_driver_validate_state_transition(driver_state_t current_state,
                                                      driver_state_t new_state);

driver_state_t generic_driver_get_state(const generic_driver_t *driver);

bool generic_driver_is_operational(const generic_driver_t *driver);

error_code_t generic_driver_cleanup(generic_driver_t *driver);

// Utility functions
const char *generic_driver_get_name(const generic_driver_t *driver);
uint32_t generic_driver_get_sample_count(const generic_driver_t *driver);

#endif // GENERIC_DRIVER_H