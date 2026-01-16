#include "simulation/generic_controller.h"
#include "simulation/generic_driver.h"
#include "core/common/utils.h"
#include <string.h>

#define MAX_ERRORS_BEFORE_EMERGENCY 10

// State transition validation table
static const bool controller_state_transitions[CONTROLLER_STATE_COUNT][CONTROLLER_STATE_COUNT] = {
    // From\To       UNINIT   IDLE   ARMED   SAMPLING   PROCESS   STORING   ERROR   EMERGENCY
    /* UNINIT */ {true, true, false, false, false, false, true, false},
    /* IDLE */ {false, true, true, true, false, false, true, true},
    /* ARMED */ {false, true, true, true, false, false, true, true},
    /* SAMPLING */ {false, true, false, true, true, false, true, true},
    /* PROCESS */ {false, false, false, false, true, true, true, true},
    /* STORING */ {false, true, false, false, false, true, true, true},
    /* ERROR */ {true, false, false, false, false, false, true, false},
    /* EMERGENCY */ {true, false, false, false, false, false, true, true}};

// Validate state indices at compile time
_Static_assert(sizeof(controller_state_transitions) / sizeof(controller_state_transitions[0]) == CONTROLLER_STATE_COUNT,
               "Controller state transition table row count mismatch");
_Static_assert(sizeof(controller_state_transitions[0]) / sizeof(controller_state_transitions[0][0]) == CONTROLLER_STATE_COUNT,
               "Controller state transition table column count mismatch");

// Event handlers for each state
static error_code_t handle_event_in_state(generic_controller_t *controller,
                                          controller_event_t event);

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
                                     size_t buffer_capacity)
{
    if (!controller || !name || !driver || !ops || !sample_buffer || !process_buffer)
    {
        return ERROR_NULL_POINTER;
    }

    if (buffer_capacity == 0 || ops->sample_size == 0 || ops->processed_size == 0)
    {
        return ERROR_INVALID_ARGUMENT;
    }

    // Initialize controller structure
    controller->controller_id = id;
    // Use safe string copy function
    strncpy(controller->controller_name, name, sizeof(controller->controller_name) - 1);
    controller->controller_name[sizeof(controller->controller_name) - 1] = '\0';

    controller->ops = ops;
    controller->driver = driver;
    controller->storage = storage;
    controller->config = config;
    controller->owns_driver = owns_driver;
    controller->is_operational = true;

    // Use provided buffers (no dynamic allocation)
    controller->sample_buffer = sample_buffer;
    controller->process_buffer = process_buffer;
    controller->buffer_capacity = buffer_capacity;

    // Initialize counters
    controller->sample_count = 0;
    controller->total_samples = 0;
    controller->last_sample_time = 0;
    controller->error_count = 0;

    // Set initial state
    controller->state = CONTROLLER_STATE_UNINITIALIZED;

    // Call controller-specific initialization
    if (controller->ops->init)
    {
        error_code_t result = controller->ops->init(controller, config);
        if (result != ERROR_NONE)
        {
            controller->state = CONTROLLER_STATE_ERROR;
            controller->is_operational = false;
            return result;
        }
    }

    // Transition to IDLE state
    return generic_controller_set_state(controller, CONTROLLER_STATE_IDLE);
}

error_code_t generic_controller_handle_event(generic_controller_t *controller,
                                             controller_event_t event)
{
    if (!controller)
    {
        return ERROR_NULL_POINTER;
    }

    if (!controller->is_operational && event != EVENT_RESET)
    {
        return ERROR_HARDWARE_FAILURE;
    }

    // Call controller-specific event handler if provided
    if (controller->ops->handle_event)
    {
        error_code_t result = controller->ops->handle_event(controller, event);
        if (result != ERROR_NOT_IMPLEMENTED)
        {
            return result;
        }
        // If handler returns NOT_IMPLEMENTED, fall through to default handling
    }

    // Default event handling based on current state
    return handle_event_in_state(controller, event);
}

static error_code_t handle_event_in_state(generic_controller_t *controller,
                                          controller_event_t event)
{
    error_code_t result = ERROR_NONE; // Initialize result

    switch (controller->state)
    {
    case CONTROLLER_STATE_IDLE:
        switch (event)
        {
        case EVENT_ARM:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_ARMED);
            break;
        case EVENT_START_SAMPLING:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_SAMPLING);
            break;
        case EVENT_EMERGENCY_STOP:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_EMERGENCY);
            break;
        default:
            result = ERROR_UNEXPECTED_STATE;
        }
        break;

    case CONTROLLER_STATE_ARMED:
        switch (event)
        {
        case EVENT_DEPLOY:
            // Would trigger deployment mechanism
            // For now, just transition to sampling
            result = generic_controller_set_state(controller, CONTROLLER_STATE_SAMPLING);
            break;
        case EVENT_RESET:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_IDLE);
            break;
        case EVENT_EMERGENCY_STOP:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_EMERGENCY);
            break;
        default:
            result = ERROR_UNEXPECTED_STATE;
        }
        break;

    case CONTROLLER_STATE_SAMPLING:
        switch (event)
        {
        case EVENT_STOP_SAMPLING:
            // Process any remaining samples before stopping
            if (controller->sample_count > 0)
            {
                result = generic_controller_set_state(controller, CONTROLLER_STATE_PROCESSING);
            }
            else
            {
                result = generic_controller_set_state(controller, CONTROLLER_STATE_IDLE);
            }
            break;
        case EVENT_PROCESS_DATA:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_PROCESSING);
            break;
        case EVENT_EMERGENCY_STOP:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_EMERGENCY);
            break;
        default:
            // Execute sampling cycle for other events or timer ticks
            result = generic_controller_execute_sampling_cycle(controller);
        }
        break;

    case CONTROLLER_STATE_PROCESSING:
        switch (event)
        {
        case EVENT_STORE_DATA:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_STORING);
            break;
        case EVENT_EMERGENCY_STOP:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_EMERGENCY);
            break;
        default:
            result = ERROR_UNEXPECTED_STATE;
        }
        break;

    case CONTROLLER_STATE_STORING:
        switch (event)
        {
        case EVENT_STOP_SAMPLING:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_IDLE);
            break;
        case EVENT_EMERGENCY_STOP:
            result = generic_controller_set_state(controller, CONTROLLER_STATE_EMERGENCY);
            break;
        default:
            // Store current batch
            // In real implementation, would call storage interface
            controller->sample_count = 0; // Clear buffer after storing
            result = generic_controller_set_state(controller, CONTROLLER_STATE_IDLE);
        }
        break;

    case CONTROLLER_STATE_EMERGENCY:
        switch (event)
        {
        case EVENT_RESET:
            // Attempt to reset system
            if (controller->driver)
            {
                generic_driver_cleanup(controller->driver);
            }
            controller->is_operational = false; // Manual intervention required
            break;
        default:
            // Most events ignored in emergency state
            result = ERROR_NONE;
        }
        break;

    default:
        result = ERROR_UNEXPECTED_STATE;
    }

    return result;
}

error_code_t generic_controller_execute_sampling_cycle(generic_controller_t *controller)
{
    if (!controller)
    {
        return ERROR_NULL_POINTER;
    }

    if (controller->state != CONTROLLER_STATE_SAMPLING)
    {
        return ERROR_UNEXPECTED_STATE;
    }

    // Check if buffer is full
    if (controller->sample_count >= controller->buffer_capacity)
    {
        // 1. Trigger processing event
        error_code_t process_result = generic_controller_handle_event(controller, EVENT_PROCESS_DATA);

        // 2. CRITICAL: If we successfully switched to PROCESSING, we MUST return immediately.
        // We cannot take another sample because there is no room in the buffer.
        if (process_result == ERROR_NONE)
        {
            return ERROR_CONTROLLER_BUSY; // Signal that we are busy dumping data
        }

        // If the event failed (e.g., transition invalid), return that error
        return process_result;
    }

    // Calculate buffer position for new sample
    size_t sample_offset = controller->sample_count * controller->ops->sample_size;
    void *sample_ptr = (uint8_t *)controller->sample_buffer + sample_offset;

    // Command driver to take sample
    error_code_t driver_result = generic_driver_take_sample(controller->driver, sample_ptr);

    if (driver_result == ERROR_NONE)
    {
        controller->sample_count++;
        controller->total_samples++;

        // Process sample immediately if controller provides processing function
        if (controller->ops->process)
        {
            size_t process_offset = (controller->sample_count - 1) * controller->ops->processed_size;
            void *process_ptr = (uint8_t *)controller->process_buffer + process_offset;

            error_code_t process_result = controller->ops->process(controller, sample_ptr, process_ptr);

            if (process_result == ERROR_NONE && controller->ops->flag)
            {
                uint32_t flags = 0;
                controller->ops->flag(controller, process_ptr, &flags);

                // Store flags with processed data (assuming processed data structure has flags field)
                // This is implementation-specific - specific controllers should handle this
                // For now, we assume the flag function modifies the processed data in place
            }

            if (process_result != ERROR_NONE)
            {
                controller->error_count++;
            }
        }
    }
    else
    {
        controller->error_count++;
        // Consider if this should trigger emergency stop
        if (controller->error_count > MAX_ERRORS_BEFORE_EMERGENCY)
        {
            generic_controller_handle_event(controller, EVENT_EMERGENCY_STOP);
        }
    }

    return driver_result;
}

error_code_t generic_controller_set_state(generic_controller_t *controller,
                                          controller_state_t new_state)
{
    if (!controller)
    {
        return ERROR_NULL_POINTER;
    }

    // Validate state transition
    error_code_t valid = generic_controller_validate_state_transition(controller->state, new_state);
    if (valid != ERROR_NONE)
    {
        return valid;
    }

    // Execute state exit actions
    switch (controller->state)
    {
    case CONTROLLER_STATE_SAMPLING:
        // Stop any ongoing sampling
        break;
    case CONTROLLER_STATE_PROCESSING:
        // Complete or abort processing
        break;
    default:
        break;
    }

    // Update state
    controller->state = new_state;

    // Execute state entry actions
    switch (new_state)
    {
    case CONTROLLER_STATE_SAMPLING:
        controller->sample_count = 0; // Reset buffer for new sampling session
        break;
    case CONTROLLER_STATE_PROCESSING:
        // Begin processing current buffer
        break;
    case CONTROLLER_STATE_STORING:
        // Begin storing processed data
        break;
    case CONTROLLER_STATE_EMERGENCY:
        // Emergency stop all operations
        controller->is_operational = false;
        if (controller->driver)
        {
            generic_driver_emergency_stop(controller->driver);
        }
        break;
    case CONTROLLER_STATE_ERROR:
        controller->is_operational = false;
        break;
    default:
        break;
    }

    return ERROR_NONE;
}

error_code_t generic_controller_validate_state_transition(controller_state_t current_state,
                                                          controller_state_t new_state)
{
    if (current_state >= CONTROLLER_STATE_COUNT || new_state >= CONTROLLER_STATE_COUNT)
    {
        return ERROR_INVALID_ARGUMENT;
    }

    if (!controller_state_transitions[current_state][new_state])
    {
        return ERROR_UNEXPECTED_STATE;
    }

    return ERROR_NONE;
}

error_code_t generic_controller_cleanup(generic_controller_t *controller)
{
    if (!controller)
    {
        return ERROR_NULL_POINTER;
    }

    // Ensure we're in a safe state
    if (controller->state == CONTROLLER_STATE_SAMPLING ||
        controller->state == CONTROLLER_STATE_PROCESSING)
    {
        generic_controller_set_state(controller, CONTROLLER_STATE_IDLE);
    }

    // Call controller-specific cleanup
    if (controller->ops->cleanup)
    {
        controller->ops->cleanup(controller);
    }

    // Clean up driver ONLY if we own it
    if (controller->driver && controller->owns_driver)
    {
        generic_driver_cleanup(controller->driver);
    }
    controller->driver = NULL;

    // Reset controller state
    controller->state = CONTROLLER_STATE_UNINITIALIZED;
    controller->ops = NULL;
    controller->config = NULL;
    controller->storage = NULL;
    controller->sample_buffer = NULL;
    controller->process_buffer = NULL;
    controller->buffer_capacity = 0;
    controller->sample_count = 0;
    controller->total_samples = 0;
    controller->error_count = 0;
    controller->owns_driver = false;
    controller->is_operational = false;

    return ERROR_NONE;
}

// Query functions
controller_state_t generic_controller_get_state(const generic_controller_t *controller)
{
    if (!controller)
    {
        return CONTROLLER_STATE_ERROR;
    }
    return controller->state;
}

const char *generic_controller_get_name(const generic_controller_t *controller)
{
    if (!controller)
    {
        return "INVALID_CONTROLLER";
    }
    return controller->controller_name;
}

uint32_t generic_controller_get_sample_count(const generic_controller_t *controller)
{
    if (!controller)
    {
        return 0;
    }
    return controller->sample_count;
}

uint64_t generic_controller_get_total_samples(const generic_controller_t *controller)
{
    if (!controller)
    {
        return 0;
    }
    return controller->total_samples;
}

uint32_t generic_controller_get_error_count(const generic_controller_t *controller)
{
    if (!controller)
    {
        return 0;
    }
    return controller->error_count;
}

bool generic_controller_is_operational(const generic_controller_t *controller)
{
    if (!controller)
    {
        return false;
    }
    return controller->is_operational;
}