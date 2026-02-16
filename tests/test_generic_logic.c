/* AI GENERATED */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "flight_software/controllers/generic_controller.h"
#include "flight_software/drivers/generic_driver.h"
#include "core/common/errors.h"

/* ============================================================
 * GLOBAL TEST CONFIGURATION
 * ============================================================ */
#define TEST_BUFFER_CAPACITY 5
#define MAX_EMERGENCY_ERRORS 10

// --- FIX 1: DEFINE STRUCT GLOBALLY TO ENSURE CONSISTENT SIZE ---
typedef struct
{
    uint8_t value;
    uint16_t flags;
} processed_data_t;

/* ============================================================
 * MOCK DRIVER IMPLEMENTATIONS
 * ============================================================ */

/* ----- Mock Driver 1: Basic Successful Operation ----- */
static int mock_driver_samples_taken = 0;
static int mock_driver_calibrations = 0;

error_code_t mock_driver_init(generic_driver_t *driver, void *config)
{
    (void)driver; (void)config;
    mock_driver_samples_taken = 0;
    mock_driver_calibrations = 0;
    return ERROR_NONE;
}

error_code_t mock_driver_sample(generic_driver_t *driver, void *sample_buffer)
{
    (void)driver;
    *(uint8_t *)sample_buffer = 42;
    mock_driver_samples_taken++;
    return ERROR_NONE;
}

error_code_t mock_driver_calibrate(generic_driver_t *driver)
{
    (void)driver;
    mock_driver_calibrations++;
    return ERROR_NONE;
}

error_code_t mock_driver_emergency(generic_driver_t *driver)
{
    (void)driver;
    return ERROR_NONE;
}

error_code_t mock_driver_cleanup(generic_driver_t *driver)
{
    (void)driver;
    return ERROR_NONE;
}

static const driver_operations_t mock_driver_ops = {
    .init = mock_driver_init,
    .sample = mock_driver_sample,
    .calibrate = mock_driver_calibrate,
    .emergency = mock_driver_emergency,
    .cleanup = mock_driver_cleanup,
    .sample_size = sizeof(uint8_t)};

/* ----- Mock Driver 2: Fault Injection ----- */
static int fault_driver_fail_count = 0;
static int fault_driver_sample_count = 0;

error_code_t fault_driver_init(generic_driver_t *driver, void *config)
{
    (void)driver; (void)config;
    fault_driver_fail_count = 0;
    fault_driver_sample_count = 0;
    return ERROR_NONE;
}

error_code_t fault_driver_sample_alternating(generic_driver_t *driver, void *sample_buffer)
{
    (void)driver;
    fault_driver_sample_count++;

    // Fail every other sample
    if (fault_driver_sample_count % 2 == 0)
    {
        fault_driver_fail_count++;
        return ERROR_HARDWARE_FAILURE;
    }

    *(uint8_t *)sample_buffer = 100;
    return ERROR_NONE;
}

static const driver_operations_t fault_driver_alternating_ops = {
    .init = fault_driver_init,
    .sample = fault_driver_sample_alternating,
    .calibrate = NULL,
    .emergency = NULL,
    .cleanup = NULL,
    .sample_size = sizeof(uint8_t)};

/* ============================================================
 * MOCK CONTROLLER IMPLEMENTATIONS
 * ============================================================ */

error_code_t mock_process(generic_controller_t *controller,
                          const void *raw,
                          void *processed)
{
    (void)controller;
    uint8_t value = *(const uint8_t *)raw;
    *(uint8_t *)processed = value * 2;
    return ERROR_NONE;
}

error_code_t mock_process_with_flag(generic_controller_t *controller,
                                    const void *raw,
                                    void *processed)
{
    (void)controller;
    uint8_t value = *(const uint8_t *)raw;

    // Use global struct definition
    processed_data_t *data = (processed_data_t *)processed;
    data->value = value * 2;

    // Flag high values
    if (value > 100)
    {
        data->flags = 0x0002; // DATA_FLAG_SUSPECT
    }
    else
    {
        data->flags = 0x0001; // DATA_FLAG_CALIBRATED
    }

    return ERROR_NONE;
}

error_code_t mock_flag_data(generic_controller_t *controller,
                            void *data,
                            uint32_t *flags)
{
    (void)controller;
    // Simulate flagging based on data content
    uint8_t *byte_data = (uint8_t *)data;
    if (*byte_data > 150)
    {
        *flags = 0x0002;
    }
    else
    {
        *flags = 0x0001;
    }
    return ERROR_NONE;
}

static const controller_operations_t mock_controller_ops = {
    .init = NULL,
    .process = mock_process,
    .flag = NULL,
    .handle_event = NULL,
    .cleanup = NULL,
    .sample_size = sizeof(uint8_t),
    .processed_size = sizeof(uint8_t)};

static const controller_operations_t mock_controller_with_flag_ops = {
    .init = NULL,
    .process = mock_process_with_flag,
    .flag = mock_flag_data,
    .handle_event = NULL,
    .cleanup = NULL,
    .sample_size = sizeof(uint8_t),
    // --- FIX 2: USE STRUCT SIZE INSTEAD OF MANUAL MATH ---
    .processed_size = sizeof(processed_data_t) 
};

/* ============================================================
 * HELPER FUNCTIONS
 * ============================================================ */
static void print_test_header(const char *test_name)
{
    printf("\n========================================\n");
    printf("  TEST: %s\n", test_name);
    printf("========================================\n");
}

static void print_test_success(const char *test_name)
{
    printf(" %s PASSED\n", test_name);
}

/* ============================================================
 * TEST SUITES
 * ============================================================ */

void test_initialization_and_state_machine()
{
    print_test_header("Initialization and State Machine");

    uint8_t raw_buffer[TEST_BUFFER_CAPACITY];
    uint8_t processed_buffer[TEST_BUFFER_CAPACITY];
    generic_driver_t driver;
    generic_controller_t controller;

    assert(generic_driver_init(&driver, "TestDriver", 1, &mock_driver_ops, NULL, NULL) == ERROR_NONE);
    assert(generic_controller_init(&controller, "TestController", 1, &driver, false,
                                   &mock_controller_ops, NULL, NULL,
                                   raw_buffer, processed_buffer, TEST_BUFFER_CAPACITY) == ERROR_NONE);

    assert(controller.state == CONTROLLER_STATE_IDLE);
    assert(generic_controller_handle_event(&controller, EVENT_ARM) == ERROR_NONE);
    assert(controller.state == CONTROLLER_STATE_ARMED);
    assert(generic_controller_handle_event(&controller, EVENT_DEPLOY) == ERROR_NONE);
    assert(controller.state == CONTROLLER_STATE_SAMPLING);
    assert(generic_controller_set_state(&controller, CONTROLLER_STATE_ARMED) == ERROR_UNEXPECTED_STATE);

    print_test_success("Initialization and State Machine");
}

void test_sampling_and_buffer_management()
{
    print_test_header("Sampling and Buffer Management");

    uint8_t raw_buffer[TEST_BUFFER_CAPACITY];
    uint8_t processed_buffer[TEST_BUFFER_CAPACITY];
    generic_driver_t driver;
    generic_controller_t controller;

    assert(generic_driver_init(&driver, "SamplingDriver", 1, &mock_driver_ops, NULL, NULL) == ERROR_NONE);
    assert(generic_controller_init(&controller, "SamplingController", 1, &driver, false,
                                   &mock_controller_ops, NULL, NULL,
                                   raw_buffer, processed_buffer, TEST_BUFFER_CAPACITY) == ERROR_NONE);

    assert(generic_controller_handle_event(&controller, EVENT_START_SAMPLING) == ERROR_NONE);

    for (int i = 0; i < TEST_BUFFER_CAPACITY; i++)
    {
        assert(generic_controller_execute_sampling_cycle(&controller) == ERROR_NONE);
        assert(controller.sample_count == (uint32_t)(i + 1));
    }

    assert(generic_controller_execute_sampling_cycle(&controller) != ERROR_NONE);
    assert(controller.state == CONTROLLER_STATE_PROCESSING);
    assert(generic_controller_handle_event(&controller, EVENT_STORE_DATA) == ERROR_NONE);
    assert(controller.state == CONTROLLER_STATE_STORING);
    
    controller.sample_count = 0; // Simulate storage clearing buffer
    assert(generic_controller_handle_event(&controller, EVENT_STOP_SAMPLING) == ERROR_NONE);
    assert(controller.state == CONTROLLER_STATE_IDLE);

    print_test_success("Sampling and Buffer Management");
}

void test_fault_handling_and_error_recovery()
{
    print_test_header("Fault Handling and Error Recovery");

    #define LARGE_BUFFER_CAPACITY 20
    uint8_t raw_buffer[LARGE_BUFFER_CAPACITY];
    uint8_t processed_buffer[LARGE_BUFFER_CAPACITY];
    generic_driver_t fault_driver;
    generic_controller_t controller;

    assert(generic_driver_init(&fault_driver, "FaultDriver", 2, &fault_driver_alternating_ops, NULL, NULL) == ERROR_NONE);
    assert(generic_controller_init(&controller, "FaultController", 2, &fault_driver, false,
                                   &mock_controller_ops, NULL, NULL,
                                   raw_buffer, processed_buffer, LARGE_BUFFER_CAPACITY) == ERROR_NONE);

    assert(generic_controller_handle_event(&controller, EVENT_START_SAMPLING) == ERROR_NONE);

    int successful_samples = 0;
    for (int i = 0; i < 12; i++)
    {
        if (generic_controller_execute_sampling_cycle(&controller) == ERROR_NONE)
            successful_samples++;
    }

    assert(successful_samples == 6);
    assert(controller.error_count == 6);

    // Trigger emergency
    for (int i = 0; i < 8; i++)
        generic_controller_execute_sampling_cycle(&controller);

    assert(controller.state == CONTROLLER_STATE_EMERGENCY || controller.error_count >= MAX_EMERGENCY_ERRORS);
    print_test_success("Fault Handling and Error Recovery");
}

void test_calibration_and_operational_states()
{
    print_test_header("Calibration and Operational States");

    uint8_t raw_buffer[TEST_BUFFER_CAPACITY];
    uint8_t processed_buffer[TEST_BUFFER_CAPACITY];
    generic_driver_t driver;
    generic_controller_t controller;

    assert(generic_driver_init(&driver, "CalDriver", 3, &mock_driver_ops, NULL, NULL) == ERROR_NONE);
    assert(generic_controller_init(&controller, "CalController", 3, &driver, false,
                                   &mock_controller_ops, NULL, NULL,
                                   raw_buffer, processed_buffer, TEST_BUFFER_CAPACITY) == ERROR_NONE);

    assert(generic_driver_calibrate(&driver) == ERROR_NONE);
    assert(generic_driver_is_operational(&driver) == true);

    // Error injection and recovery
    assert(generic_driver_set_state(&driver, DRIVER_STATE_ERROR) == ERROR_NONE);
    uint8_t sample;
    assert(generic_driver_take_sample(&driver, &sample) == ERROR_HARDWARE_FAILURE);

    // Proper recovery sequence: ERROR -> UNINIT -> IDLE
    assert(generic_driver_set_state(&driver, DRIVER_STATE_UNINITIALIZED) == ERROR_NONE);
    assert(generic_driver_init(&driver, "CalDriver", 3, &mock_driver_ops, NULL, NULL) == ERROR_NONE);
    assert(generic_driver_is_operational(&driver) == true);

    print_test_success("Calibration and Operational States");
}

void test_multiple_independent_instances()
{
    print_test_header("Multiple Independent Instances");

    generic_driver_t driver1, driver2;
    generic_controller_t controller1, controller2;
    uint8_t buf1[5], buf2[5], proc1[5], proc2[5];

    assert(generic_driver_init(&driver1, "D1", 1, &mock_driver_ops, NULL, NULL) == ERROR_NONE);
    assert(generic_driver_init(&driver2, "D2", 2, &mock_driver_ops, NULL, NULL) == ERROR_NONE);

    assert(generic_controller_init(&controller1, "C1", 1, &driver1, false, &mock_controller_ops, NULL, NULL, buf1, proc1, 5) == ERROR_NONE);
    assert(generic_controller_init(&controller2, "C2", 2, &driver2, false, &mock_controller_ops, NULL, NULL, buf2, proc2, 5) == ERROR_NONE);

    generic_controller_handle_event(&controller1, EVENT_START_SAMPLING);
    generic_controller_handle_event(&controller2, EVENT_START_SAMPLING);

    generic_controller_execute_sampling_cycle(&controller1);
    generic_controller_execute_sampling_cycle(&controller2);
    generic_controller_execute_sampling_cycle(&controller2);

    assert(controller1.sample_count == 1);
    assert(controller2.sample_count == 2);

    print_test_success("Multiple Independent Instances");
}

void test_data_processing_and_flagging()
{
    print_test_header("Data Processing and Flagging");

    uint8_t raw_buffer[TEST_BUFFER_CAPACITY];
    
    // --- FIX 3: ALLOCATE BUFFER USING CORRECT STRUCT SIZE ---
    // Using sizeof(processed_data_t) ensures enough space regardless of padding
    uint8_t processed_buffer[TEST_BUFFER_CAPACITY * sizeof(processed_data_t)];
    
    // Clear buffer to avoid garbage data
    memset(processed_buffer, 0, sizeof(processed_buffer));

    generic_driver_t driver;
    generic_controller_t controller;

    assert(generic_driver_init(&driver, "DataDriver", 4, &mock_driver_ops, NULL, NULL) == ERROR_NONE);
    assert(generic_controller_init(&controller, "DataController", 4, &driver, false,
                                   &mock_controller_with_flag_ops, NULL, NULL,
                                   raw_buffer, processed_buffer, TEST_BUFFER_CAPACITY) == ERROR_NONE);

    assert(generic_controller_handle_event(&controller, EVENT_START_SAMPLING) == ERROR_NONE);

    for (int i = 0; i < 3; i++)
    {
        assert(generic_controller_execute_sampling_cycle(&controller) == ERROR_NONE);
    }

    // Check data integrity
    processed_data_t *data = (processed_data_t *)processed_buffer;
    
    // Driver writes 42 -> Process multiplies by 2 -> Value should be 84
    assert(data[0].value == 84);
    
    // 84 is < 100, so flag should be 0x0001 (CALIBRATED)
    // If this fails, it means the struct padding offset was calculated wrong in previous steps
    assert(data[0].flags == 0x0001);

    print_test_success("Data Processing and Flagging");
}

// int main(void)
// {
//     printf("========================================\n");
//     printf("  ENHANCED GENERIC COMPONENT TEST SUITE  \n");
//     printf("========================================\n\n");

//     test_initialization_and_state_machine();
//     test_sampling_and_buffer_management();
//     test_fault_handling_and_error_recovery();
//     test_calibration_and_operational_states();
//     test_multiple_independent_instances();
//     test_data_processing_and_flagging();

//     printf("\n========================================\n");
//     printf("  ALL TESTS PASSED SUCCESSFULLY!\n");
//     printf("========================================\n");

//     return 0;
// }