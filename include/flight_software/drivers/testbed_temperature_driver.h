
#ifndef TTD_DRIVER_H
#define TTD_DRIVER_H

#include "flight_software/flight_software_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TTD_FLAGS_IN_USE 2
#define TTD_DOUBLES_PER_SAMPLE 2 //data flags, data itself
#define TTD_SAMPLES_PER_WRITE 16
#define TTD_PERIPHERAL_COLUMNS 2 //driver state, driver error flags
#define TTD_CSV_COLUMNS_COUNT ( TTD_DOUBLES_PER_SAMPLE + TTD_PERIPHERAL_COLUMNS )

// SPI pins
#define RTD_PIN_SCK     18
#define RTD_PIN_MOSI    19
#define RTD_PIN_MISO    16
#define RTD_PIN_CS      17

// MAX31865 registers
#define MAX31865_REG_CONFIG     0x00
#define MAX31865_REG_RTD_MSB    0x01
#define MAX31865_REG_RTD_LSB    0x02
#define MAX31865_REG_FAULT      0x07
#define MAX31865_WRITE_BIT      0x80
#define MAX31865_CONFIG_3WIRE   0b11000011

// PT100 constants
#define RTD_NOMINAL_OHMS        100.0f
#define RTD_REF_OHMS            430.0f
#define RTD_ALPHA               0.00385f

typedef enum TTD_motor_e TTD_motor_e;
typedef enum TTD_error_e TTD_error_e;

enum TTD_motor_e {

    MOTOR_1_E,
    MOTOR_2_E,
    MOTOR_3_E,
    MOTOR_4_E,
    MOTOR_5_E,
    MOTOR_6_E,
    MOTOR_7_E,
    MOTOR_8_E,

TTD_DRIVER_MOTOR_COUNT
};

enum TTD_error_e {

    D_ERROR_REDUNDANT_INITIALIZATION_E,
    D_ERROR_RIDICULOUS_BOOM_EXTENSION_E,
    D_ERROR_SAMPLE_BUFFER_NPE_E,
    D_ERROR_STORAGE_BUFFER_NPE_E,
    D_ERROR_ILLEGAL_DEPLOYMENT_E,
    D_ERROR_REDUNDANT_DEPLOYMENT_E,
    D_ERROR_ILLEGAL_RETRACTION_E,
    D_ERROR_REDUNDANT_RETRACTION_E,
    D_ERROR_ILLEGAL_SAMPLE_E,

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

// /**
//  * @brief Returns a double indicating how extended the argued motor has become.
//  * @param which_motor motor_e
//  * @retval -1: error in deployment
//  * @returns positive double indicating boom extension
//  */
// double TTD_measure_extension( TTD_motor_e which_motor );

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
unsigned char TTD_process_sample( sample_t* sample );

void    rtd_driver_init(void);
int     rtd_read_raw(uint16_t* out_raw);
float   rtd_raw_to_resistance(uint16_t raw);
float   rtd_resistance_to_celsius(float resistance);

bool TTD_negative_flag( sample_t* sample );
bool TTD_positive_flag( sample_t* sample );
bool data_flag_generic_3( sample_t* sample );
bool data_flag_generic_4( sample_t* sample );

bool data_flag_generic_5( sample_t* sample );
bool data_flag_generic_6( sample_t* sample );
bool data_flag_generic_7( sample_t* sample );
bool data_flag_generic_8( sample_t* sample );

#endif
