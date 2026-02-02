#ifndef GENERIC_CONTROLLER_H
#define GENERIC_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "core/common/errors.h"

// Forward declarations only
typedef struct generic_driver generic_driver_t;
typedef struct generic_controller generic_controller_t;

// Forward declare storage_handle_t if not defined elsewhere
typedef void *storage_handle_t; // Placeholder - replace with actual type

typedef enum
{
    CONTROLLER_STATE_UNINITIALIZED = 0,
    CONTROLLER_STATE_IDLE,
    CONTROLLER_STATE_ARMED,
    CONTROLLER_STATE_SAMPLING,
    CONTROLLER_STATE_PROCESSING,
    CONTROLLER_STATE_STORING,
    CONTROLLER_STATE_ERROR,
    CONTROLLER_STATE_EMERGENCY,
    CONTROLLER_STATE_COUNT // Keep last for array sizing
} controller_state_t;

typedef enum
{
    EVENT_NONE = 0,
    EVENT_ARM,
    EVENT_DEPLOY,
    EVENT_START_SAMPLING,
    EVENT_STOP_SAMPLING,
    EVENT_PROCESS_DATA,
    EVENT_STORE_DATA,
    EVENT_EMERGENCY_STOP,
    EVENT_RESET
} controller_event_t;

// Function pointer types for controller operations
typedef error_code_t (*controller_init_func_t)(generic_controller_t *controller, void *config);
typedef error_code_t (*controller_process_func_t)(generic_controller_t *controller,
                                                  const void *raw_sample,
                                                  void *processed_data);
typedef error_code_t (*controller_flag_func_t)(generic_controller_t *controller,
                                               void *data,
                                               uint32_t *flags);
typedef error_code_t (*controller_handle_event_func_t)(generic_controller_t *controller,
                                                       controller_event_t event);
typedef error_code_t (*controller_cleanup_func_t)(generic_controller_t *controller);

typedef struct
{
    controller_init_func_t init;
    controller_process_func_t process;
    controller_flag_func_t flag;
    controller_handle_event_func_t handle_event;
    controller_cleanup_func_t cleanup;
    size_t sample_size;    // Size of raw sample
    size_t processed_size; // Size of processed data
} controller_operations_t;

struct generic_controller
{
    uint8_t controller_id;
    char controller_name[32];
    controller_state_t state;
    const controller_operations_t *ops; // Instance-specific operations

    // Owned resources
    generic_driver_t *driver; // Controller owns the driver
    storage_handle_t storage;

    // Configuration
    void *config;

    // Data buffers (pre-allocated, no malloc in flight)
    void *sample_buffer;    // For raw samples from driver
    void *process_buffer;   // For processed data
    size_t buffer_capacity; // Max samples per buffer
    uint32_t sample_count;  // Current samples in buffer

    // Statistics
    uint64_t total_samples;
    uint64_t last_sample_time;
    uint32_t error_count;

    // Ownership flags
    bool owns_driver;    // True if controller owns/created the driver
    bool is_operational; // Emergency status flag
};

// Core controller API
error_code_t generic_controller_init(generic_controller_t *controller,
                                     const char *name,
                                     uint8_t id,
                                     generic_driver_t *driver,
                                     bool owns_driver,
                                     const controller_operations_t *ops,
                                     storage_handle_t storage,
                                     void *config,
                                     void *sample_buffer,
                                     void *process_buffer,
                                     size_t buffer_capacity);

error_code_t generic_controller_handle_event(generic_controller_t *controller,
                                             controller_event_t event);

error_code_t generic_controller_execute_sampling_cycle(generic_controller_t *controller);

error_code_t generic_controller_set_state(generic_controller_t *controller,
                                          controller_state_t new_state);

error_code_t generic_controller_validate_state_transition(controller_state_t current_state,
                                                          controller_state_t new_state);

error_code_t generic_controller_cleanup(generic_controller_t *controller);

// Query functions
controller_state_t generic_controller_get_state(const generic_controller_t *controller);
const char *generic_controller_get_name(const generic_controller_t *controller);
uint32_t generic_controller_get_sample_count(const generic_controller_t *controller);
uint64_t generic_controller_get_total_samples(const generic_controller_t *controller);
uint32_t generic_controller_get_error_count(const generic_controller_t *controller);
bool generic_controller_is_operational(const generic_controller_t *controller);

#endif // GENERIC_CONTROLLER_H