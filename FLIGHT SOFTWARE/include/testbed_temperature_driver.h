
#ifndef TTD_DRIVER_H
#define TTD_DRIVER_H

#include "testbed_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TTD_FLAGS_IN_USE 3
#define TTD_DOUBLES_PER_SAMPLE 3 //data flags, raw data, converted data
#define TTD_SAMPLES_PER_WRITE 16
#define TTD_PERIPHERAL_COLUMNS 2 //driver state, driver error flags
#define TTD_CSV_COLUMNS_COUNT ( TTD_DOUBLES_PER_SAMPLE + TTD_PERIPHERAL_COLUMNS )

/*============\
PIN DEFINITIONS
\============*/
#define MOTOR_PIN 2

#define MAX_SCK_PIN 10
#define MAX_MOSI_PIN 11
#define MAX_MISO_PIN 12
#define MAX_CS_PIN 13

#define LED_PIN 25

/*==========\
SCIENCE STUFF
\==========*/
// #define RTD_NOMINAL_OHMS        100.0f
#define RTD_REF_OHMS            430.0f
#define PT100_RESISTANCE_0C   100.0   // Resistance at 0°C (ohms)
#define PT100_ALPHA           0.385   // Temperature coefficient (ohms/°C)
// #define RTD_ALPHA               0.00385f

// typedef enum TTD_motor_e TTD_motor_e;
// typedef enum TTD_error_e TTD_error_e;

enum TTD_motor_e {

    MOTOR_1_E,

TTD_MOTOR_COUNT
};

enum TTD_error_e {

    D_ERROR_REDUNDANT_INITIALIZATION_E,
    D_ERROR_RIDICULOUS_BOOM_EXTENSION_E,
    D_ERROR_SAMPLE_BUFFER_NPE_E,
    D_ERROR_STORAGE_BUFFER_NPE_E,
    D_ERROR_ILLEGAL_DEPLOYMENT_E, //*** */
    D_ERROR_REDUNDANT_DEPLOYMENT_E,
    D_ERROR_ILLEGAL_RETRACTION_E,
    D_ERROR_REDUNDANT_RETRACTION_E,
    D_ERROR_ILLEGAL_SAMPLE_E, //*** */

    D_FAULT_HIGH_THRESH_E,
    D_FAULT_LOW_THRESH_E,
    D_FAULT_REF_IN_LOW_E,
    D_FAULT_REF_IN_HIGH_E,
    D_FAULT_RTD_IN_LOW_E,
    D_FAULT_OVER_UNDER_VOLTAGE_E,

    D_FAULT_NEGATIVE_DISCRIMINANT,

TTD_ERROR_COUNT
};

/**
 * @brief Initializes the instrument driver. Injects a reference to the sample buffer into the driver.
 * @note Leaves the initialized instrument in the DEPLOYMENT_E state.
 * @param instrument instrument_t* allocated by calling code
 * @param target_deployment_inches int
 * @param sample_buffer sample_t* allocated by calling code
 * @param storage_buffer csv_t* allocated by calling code
 */
void TTD_initialize_driver( instrument_t* instrument,
                    int target_deployment_inches, 
                    sample_t* sample_buffer,
                    csv_t* storage_buffer );

/**
 * @brief Deploys the instrumentation this driver is responsible for.
 * @note Leaves the instrument in the READY_E state.
 */
int TTD_deploy_instrumentation(void);

/**
 * @brief Retracts the driven instrumentation
 * @note leaves the driver in DEPLOYMENT_E state
 * @retval -1: illegal state
 * @retval -2: instrumentation was not deployed to begin with
 * @retval 0: still retracting at least one motor
 * @retval 1: all motors retracted
 */
int TTD_retract_instrumentation(void);


/**
 * @brief Returns a double indicating how extended the argued motor has become.
 * @retval -1: error in deployment
 * @returns positive double indicating boom extension
 */
double TTD_measure_extension(void);

/**
 * @brief Samples the instruentation into the sample buffer.
 * @retval NULL: failure (Driver in wrong state, malloc() NPE,)
 * @retval sample_t*: success
 */
void TTD_sample(void);

/**
 * @brief Processes the data in the sample buffer. Returns an 8-flag byte to characterize the sample.
 * 
 * 1. FLAG ONE
 * 2. FLAG TWO
 * 3. FLAG THREE
 * 4. FLAG FOUR
 * 5. FLAG FIVE
 * 6. FLAG SIX
 * 7. FLAG SEVEN
 * 8. FLAG EIGHT
 */
void TTD_process_sample( sample_t* sample );

bool TTD_cold_flag( sample_t* sample );
bool TTD_warm_flag( sample_t* sample );
bool TTD_hot_flag( sample_t* sample );
bool TTD_unused_flag_3( sample_t* sample );

bool TTD_unused_flag_4( sample_t* sample );
bool TTD_unused_flag_5( sample_t* sample );
bool TTD_unused_flag_6( sample_t* sample );
bool TTD_unused_flag_7( sample_t* sample );

#endif
