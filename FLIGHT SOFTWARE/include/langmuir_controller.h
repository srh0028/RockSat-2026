
#ifndef LP_CONTROLLER_H
#define LP_CONTROLLER_H

#include "langmuir_driver.h"
#include "langmuir_types.h"

/*============\
PIN DEFINITIONS
\============*/
#define LVL_HIGH 1
#define LVL_LOW 0

#define UART_TX_PIN 1
#define UART_RX_PIN 2
#define UART_BAUD_RATE 230400

#define RECORD_ENABLE_PIN 22

#define TE1_PIN 31
#define TE2_PIN 32

#define CONVERTERS_SCK_PIN 4
#define CONVERTERS_MOSI_PIN 5

#define ADC_CHANNELS_IN_USE 5
#define ADC_MISO_0_PIN 6
#define ADC_MISO_1_PIN 7
#define ADC_MISO_2_PIN 9
#define ADC_MISO_3_PIN 10
#define ADC_MISO_4_PIN 11
#define ADC_CNV_PIN 16
#define ADC_BUSY_PIN 17
#define ADC_CS_PIN 19

#define DAC_CS_PIN 34

#define LED_PIN 25

#define MOTOR_COUNT 6
#define MOTOR_PWM_PIN 20
#define MOTOR_PWM_DEPLOYMENT 100 //microseconds
#define MOTOR_PWM_RETRACTION 5500 //microseconds
#define MOTOR_PWM_STATIONARY 1500 //microseconds

#define PWM_CLOCK_DIVIDER   125.0f  // 125MHz / 125 = 1MHz (1us per count)
#define PWM_PERIOD_US       20000   // 20ms period (50Hz)

//motor testing pins
#define TEST_LIMIT_SWITCH_PIN 15
#define TEST_MOTOR_PIN 12

//limit switches?
#define MOTOR_PIN_0 24
#define MOTOR_PIN_1 25
#define MOTOR_PIN_2 26
#define MOTOR_PIN_3 27
#define MOTOR_PIN_4 29
#define MOTOR_PIN_5 21

/* all caps means peripheral to this scope
TIMESTAMP millis()
TIMED EVENT
CONTROLLER ERRORS
driver errors
adc housekeeping
p1V
p1I
p2V
p2I
*/
#define LPC_PERIPHERAL_COLUMNS 3
#define LPC_DRIVERS_UTILIZED 1
#define LPC_OUTPUT_FILE_NAME "Lang"
#define MAX_FILENAME_LENGTH 32
#define LPC_STORAGE_COLUMNS ( LPD_CSV_COLUMNS_COUNT + LPC_PERIPHERAL_COLUMNS )
#define LPC_DATA_BUFFER_SIZE ( LPC_STORAGE_COLUMNS * LPD_SAMPLES_PER_WRITE )

// #define ERROR_LED_BIT0 16  // Physical pin 21 - Least significant bit (1)
// #define ERROR_LED_BIT1 17  // Physical pin 22 - Middle bit (2)
// #define ERROR_LED_BIT2 18  // Physical pin 24 - Most significant bit (4)
// #define ERROR_LED_BIT3 19 //physical pin 25

// enum LED_CODE_E {

//     C_CODE_SD_INITIALIZED_E, //SD initialized
//     C_CODE_SD_INITALIZATION_FAILED_E, //SD not initialized
//     C_CODE_SAMPLE_CYCLE_E,
//     C_CODE_OPENING_SD_E,
//     C_CODE_CLOSING_SD_E,
//     C_CODE_OPENING_FAILED_E,
//     C_CODE_GENERATING_FILENAME_E,
//     C_CODE_CHECKING_FILENAME_E,
//     C_CODE_FILENAME_USED_E,
//     C_CODE_FILENAME_AVAILABLE_E,
//     C_CODE_11_E,
//     C_CODE_12_E,
//     C_CODE_13_E,
//     C_CODE_14_E,
//     C_CODE_15_E,

// LED_CODE_COUNT
// };

// void display_LED_code( LED_CODE_E which_code );

enum LPC_errors_e {

    C_ERROR_ENVIRONMENT_FALLTHROUGH_E,
    C_ERROR_TIMED_EVENT_FALLTHROUGH_E,
    C_ERROR_DEPLOYMENT_WHILE_UNINITIALIZED_E,
    C_ERROR_UNPREPARED_TO_SAMPLE_E,
    C_ERROR_STORAGE_INITIALIZATION_FAILURE_E,
    C_ERROR_SAMPLE_STORAGE_FORMAT_MISMATCH_E,

LPC_ERROR_COUNT
};

void initialize_pins(void);

/**
 * @brief Initialize a controller with the given number of instruments.
 * @returns controller state after initialization
 */
controller_state_e LPC_initialize(void);

/**
 * @brief Reads in a timed event from the simulation.
 * @retval 1 if a timed event is active
 * @retval 0 if a timed event is not active
 */
int LPC_read_in_sim_timed_event(void);

/**
 * @brief Reads in a timed event in flight.
 * @retval 0 TE pin = logic low
 * @retval 1 TE pin = logic high
 */
int LPC_read_in_flight_timed_event(void);

void LPC_deploy(void);

void LPC_retract(void);

/**
 * @brief Performs one cycle of sampling.
 */
void LPC_sample_cycle(void);

void LPC_timed_event_1_handler(void);
void LPC_timed_event_2_handler(void);

void save_buffer_to_SD(csv_t* csv);

// void deployment_blinker(void);

// void retraction_blinker(void);

/**
 * @brief Turns an array of booleans into a double which represents them all bitwise.
 * @param bool_array bool* pointer to the bool array
 * @param size int size of the bool array
 */
int crunch_flags(bool bool_array[], int size);

void write_sample_to_csv(csv_t* storage_buffer, sample_t* sample, int first_column_index);

void generate_next_filename(char* buffer, int buffer_size, const char* base_name);

void initialize_PWM( int which_pin );

void pwm_servo_set_pulse_us( uint which_pin, uint pulse_width_us );

#endif
