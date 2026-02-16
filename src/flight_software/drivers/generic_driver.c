#include "flight_software/drivers/generic_driver.h"
#include "core/common/utils.h"
#include "simulation/mounts/generic_mount.h"
#include <string.h>

// State transition validation table
static const bool state_transitions[DRIVER_STATE_COUNT][DRIVER_STATE_COUNT] = {
    // From\To      UNINIT   IDLE   SAMPLING   CALIB   ERROR
    /* UNINIT */ {true, true, false, false, true},
    /* IDLE */ {false, true, true, true, true},
    /* SAMPLING */ {false, true, true, false, true},
    /* CALIB */ {false, true, false, true, true},
    /* ERROR */ {true, false, false, false, true}};

// Validate state indices at compile time
_Static_assert(sizeof(state_transitions) / sizeof(state_transitions[0]) == DRIVER_STATE_COUNT,
               "State transition table row count mismatch");
_Static_assert(sizeof(state_transitions[0]) / sizeof(state_transitions[0][0]) == DRIVER_STATE_COUNT,
               "State transition table column count mismatch");

error_code_t generic_driver_init(generic_driver_t *driver,
                                 const char *name,
                                 uint8_t id,
                                 const driver_operations_t *ops,
                                 void *config,
                                 void *hardware_context)
{
    if (!driver || !name || !ops)
    {
        return ERROR_NULL_POINTER;
    }

    if (ops->sample_size == 0)
    {
        return ERROR_INVALID_ARGUMENT;
    }

    // Initialize basic driver info
    driver->driver_id = id;
    strncpy(driver->driver_name, name, sizeof(driver->driver_name) - 1);
    driver->driver_name[sizeof(driver->driver_name) - 1] = '\0';

    driver->ops = ops;
    driver->config = config;
    driver->hardware_context = hardware_context;
    driver->sample_count = 0;
    driver->last_sample_time = 0;
    driver->is_operational = true;

    // Set initial state
    driver->state = DRIVER_STATE_UNINITIALIZED;

    // Call driver-specific initialization
    if (driver->ops->init)
    {
        error_code_t result = driver->ops->init(driver, config);
        if (result != ERROR_NONE)
        {
            driver->state = DRIVER_STATE_ERROR;
            driver->is_operational = false;
            return result;
        }
    }

    // Transition to IDLE state
    error_code_t state_result = generic_driver_set_state(driver, DRIVER_STATE_IDLE);
    if (state_result != ERROR_NONE)
    {
        driver->is_operational = false;
        return state_result;
    }

    return ERROR_NONE;
}

error_code_t generic_driver_take_sample(generic_driver_t *driver,
                                        void *sample_buffer)
{
    if (!driver || !sample_buffer)
    {
        return ERROR_NULL_POINTER;
    }

    if (!driver->is_operational)
    {
        return ERROR_HARDWARE_FAILURE;
    }

    // Validate state transition
    error_code_t state_result = generic_driver_set_state(driver, DRIVER_STATE_SAMPLING);
    if (state_result != ERROR_NONE)
    {
        return state_result;
    }

    // Call driver-specific sampling function
    if (driver->ops->sample)
    {
        error_code_t result = driver->ops->sample(driver, sample_buffer);

        if (result == ERROR_NONE)
        {
            driver->sample_count++;
            // Could add timestamp here if we have a time source
        }

        // Return to IDLE state regardless of result
        generic_driver_set_state(driver, DRIVER_STATE_IDLE);
        return result;
    }

    // No sample function implemented
    generic_driver_set_state(driver, DRIVER_STATE_ERROR);
    driver->is_operational = false;
    return ERROR_NOT_IMPLEMENTED;
}

error_code_t generic_driver_calibrate(generic_driver_t *driver)
{
    if (!driver)
    {
        return ERROR_NULL_POINTER;
    }

    if (!driver->is_operational)
    {
        return ERROR_HARDWARE_FAILURE;
    }

    // Validate state transition
    error_code_t state_result = generic_driver_set_state(driver, DRIVER_STATE_CALIBRATING);
    if (state_result != ERROR_NONE)
    {
        return state_result;
    }

    // Call driver-specific calibration function
    if (driver->ops->calibrate)
    {
        error_code_t result = driver->ops->calibrate(driver);

        // Return to IDLE state
        generic_driver_set_state(driver, DRIVER_STATE_IDLE);
        return result;
    }

    // No calibration function implemented
    generic_driver_set_state(driver, DRIVER_STATE_ERROR);
    driver->is_operational = false;
    return ERROR_NOT_IMPLEMENTED;
}

error_code_t generic_driver_emergency_stop(generic_driver_t *driver)
{
    if (!driver)
    {
        return ERROR_NULL_POINTER;
    }

    // Set emergency flag
    driver->is_operational = false;

    // Call driver-specific emergency function if available
    if (driver->ops->emergency)
    {
        error_code_t result = driver->ops->emergency(driver);
        if (result != ERROR_NONE)
        {
            // Log error but continue with generic emergency handling
        }
    }

    // Force transition to error state
    driver->state = DRIVER_STATE_ERROR;

    return ERROR_NONE;
}

error_code_t generic_driver_set_state(generic_driver_t *driver,
                                      driver_state_t new_state)
{
    if (!driver)
    {
        return ERROR_NULL_POINTER;
    }

    // Validate state transition
    error_code_t valid = generic_driver_validate_state_transition(driver->state, new_state);
    if (valid != ERROR_NONE)
    {
        return valid;
    }

    // Special handling for error state
    if (new_state == DRIVER_STATE_ERROR)
    {
        driver->is_operational = false;
    }

    driver->state = new_state;
    return ERROR_NONE;
}

error_code_t generic_driver_validate_state_transition(driver_state_t current_state,
                                                      driver_state_t new_state)
{
    if (current_state >= DRIVER_STATE_COUNT || new_state >= DRIVER_STATE_COUNT)
    {
        return ERROR_INVALID_ARGUMENT;
    }

    if (!state_transitions[current_state][new_state])
    {
        return ERROR_UNEXPECTED_STATE;
    }

    return ERROR_NONE;
}

driver_state_t generic_driver_get_state(const generic_driver_t *driver)
{
    if (!driver)
    {
        return DRIVER_STATE_ERROR;
    }
    return driver->state;
}

bool generic_driver_is_operational(const generic_driver_t *driver)
{
    if (!driver)
    {
        return false;
    }
    return driver->is_operational;
}

error_code_t generic_driver_cleanup(generic_driver_t *driver)
{
    if (!driver)
    {
        return ERROR_NULL_POINTER;
    }

    // Call driver-specific cleanup
    if (driver->ops->cleanup)
    {
        driver->ops->cleanup(driver);
    }

    // Reset driver state
    driver->state = DRIVER_STATE_UNINITIALIZED;
    driver->ops = NULL;
    driver->config = NULL;
    driver->hardware_context = NULL;
    driver->sample_count = 0;
    driver->is_operational = false;

    return ERROR_NONE;
}

// Utility functions
const char *generic_driver_get_name(const generic_driver_t *driver)
{
    if (!driver)
    {
        return "INVALID_DRIVER";
    }
    return driver->driver_name;
}

uint32_t generic_driver_get_sample_count(const generic_driver_t *driver)
{
    if (!driver)
    {
        return 0;
    }
    return driver->sample_count;
}

// Addition to link with simulator
error_code_t generic_sample(generic_driver_t *driver, void *sample_buffer)
{
    if (!driver || !sample_buffer)
        return ERROR_NULL_POINTER;

    if (!driver->hardware_context)
        return ERROR_HARDWARE_FAILURE;

    instrument_mount_t *mount =
        (instrument_mount_t *)driver->hardware_context;

    // Modify the mount directly:
    mount->pins[PIN_GENERIC_E] = 68;

    // Could also use update_pin()
    // update_pin(
    //     INSTRUMENT_GENERIC_E, // If needed, but better avoided (see below)
    //     PIN_GENERIC_E,
    //     68);

    double reading = mount->instrument_reading();

    *(double *)sample_buffer = reading;

    return ERROR_NONE;
}
