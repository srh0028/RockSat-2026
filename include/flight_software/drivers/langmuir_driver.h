
#ifndef LPD_DRIVER_H
#define LPD_DRIVER_H

#include "langmuir_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LPD_DOUBLES_PER_SAMPLE 5 //housekeeping, p1v, p1i, p2v, p2i
#define LPD_SAMPLES_PER_WRITE 32
#define LPD_PERIPHERAL_COLUMNS 1 //driver error flags
#define LPD_CSV_COLUMNS_COUNT ( LPD_DOUBLES_PER_SAMPLE + LPD_PERIPHERAL_COLUMNS )

/*============\
PIN DEFINITIONS
\============*/


/*==========\
SCIENCE STUFF
\==========*/

enum LPD_error_e {

    D_ERROR_REDUNDANT_INITIALIZATION_E,
    D_ERROR_SAMPLE_BUFFER_NPE_E,
    D_ERROR_STORAGE_BUFFER_NPE_E,
    D_ERROR_ILLEGAL_DEPLOYMENT_E,
    D_ERROR_REDUNDANT_DEPLOYMENT_E,
    D_ERROR_ILLEGAL_RETRACTION_E,
    D_ERROR_REDUNDANT_RETRACTION_E,
    D_ERROR_ILLEGAL_SAMPLE_E,

LPD_ERROR_COUNT
};

/**
 * @brief Initializes the instrument driver. Injects a reference to the sample buffer into the driver.
 * @note Leaves the initialized instrument in the DEPLOYMENT_E state.
 * @param instrument instrument_t* allocated by calling code
 * @param target_deployment_inches int
 * @param sample_buffer sample_t* allocated by calling code
 * @param storage_buffer csv_t* allocated by calling code
 */
void LPD_initialize_driver( instrument_t* instrument
                    sample_t* sample_buffer,
                    csv_t* storage_buffer );

/**
 * @brief Deploys the instrumentation this driver is responsible for.
 * @note Leaves the instrument in the READY_E state.
 */
void LPD_deploy_instrumentation(void);

/**
 * @brief Retracts the driven instrumentation
 * @note leaves the driver in DEPLOYMENT_E state
 * @retval -1: illegal state
 * @retval -2: instrumentation was not deployed to begin with
 * @retval 0: still retracting at least one motor
 * @retval 1: all motors retracted
 */
int LPD_retract_instrumentation(void);

/**
 * @brief Returns a double indicating how extended the argued motor has become.
 * @retval -1: error in deployment
 * @returns positive double indicating boom extension
 */
bool LPD_measure_extension(void);

/**
 * @brief Samples the instruentation into the sample buffer.
 * @retval NULL: failure (Driver in wrong state, malloc() NPE,)
 * @retval sample_t*: success
 */
void LPD_sample(void);

#endif
