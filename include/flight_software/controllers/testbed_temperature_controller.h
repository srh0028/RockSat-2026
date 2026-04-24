
#ifndef TT_CONTROLLER_H
#define TT_CONTROLLER_H

#include "testbed_temperature_driver.h"
#include "testbed_types.h"
// #include "simulation/real_time_engine.h"

/*============\
PIN DEFINITIONS
\============*/
#define SD_MISO_PIN 4
#define SD_CS_PIN 5
#define SD_SCK_PIN 6
#define SD_MOSI_PIN 7

/* all caps means peripheral to this scope
TIMESTAMP millis()
TIMESTAMP delta_time
TIMESTAMP mission_seconds
CONTROLLER STATE
CONTROLLER ERRORS
driver state
driver errors
flags
data
*/
#define TTC_PERIPHERAL_COLUMNS 5
#define TTC_DRIVERS_UTILIZED 1
#define TTC_OUTPUT_FILE_NAME "Test"
#define TTC_STORAGE_COLUMNS ( TTD_CSV_COLUMNS_COUNT + TTC_PERIPHERAL_COLUMNS )
#define TTC_DATA_BUFFER_SIZE ( TTC_STORAGE_COLUMNS * TTD_SAMPLES_PER_WRITE )

#define ERROR_LED_BIT0 16  // Physical pin 21 - Least significant bit (1)
#define ERROR_LED_BIT1 17  // Physical pin 22 - Middle bit (2)
#define ERROR_LED_BIT2 18  // Physical pin 24 - Most significant bit (4)
#define ERROR_LED_BIT3 19 //physical pin 25

enum LED_CODE_E {

    C_CODE_SD_INITIALIZED_E, //SD initialized
    C_CODE_SD_INITALIZATION_FAILED_E, //SD not initialized
    C_CODE_SAMPLE_CYCLE_E,
    C_CODE_OPENING_SD_E,
    C_CODE_CLOSING_SD_E,
    C_CODE_OPENING_FAILED_E,
    C_CODE_GENERATING_FILENAME_E,
    C_CODE_CHECKING_FILENAME_E,
    C_CODE_FILENAME_USED_E,
    C_CODE_FILENAME_AVAILABLE_E,
    C_CODE_11_E,
    C_CODE_12_E,
    C_CODE_13_E,
    C_CODE_14_E,
    C_CODE_15_E,

LED_CODE_COUNT
};

void display_LED_code( LED_CODE_E which_code );

// typedef enum TTC_errors_e TTC_errors_e;
enum TTC_errors_e {

    C_ERROR_ENVIRONMENT_FALLTHROUGH_E,
    C_ERROR_TIMED_EVENT_FALLTHROUGH_E,
    C_ERROR_DEPLOYMENT_WHILE_UNINITIALIZED_E,
    C_ERROR_UNPREPARED_TO_SAMPLE_E,

TTC_ERROR_COUNT
};

// /**
//  * @brief Entry point for the controller. Use this to link up the controller struct
//  */
// void setup(void);

// /**
//  * @brief Main looping function for the microcontroller
//  * @note Wee haw!
//  * @todo ADD A GIANT ERROR MESSAGE IN HERE
//  */
// void loop(void);

/**
 * @brief Initialize a controller with the given number of instruments.
 * @returns controller state after initialization
 */
controller_state_e TTC_initialize(void);

/**
 * @brief Reads in a timed event from the simulation.
 * @retval 1 if a timed event is active
 * @retval 0 if a timed event is not active
 */
int TTC_read_in_sim_timed_event(void);

/**
 * @brief Reads in a timed event in flight.
 * @retval 0 TE pin = logic low
 * @retval 1 TE pin = logic high
 */
int TTC_read_in_flight_timed_event(void);

void TTC_deploy(void);

void TTC_retract(void);

/**
 * @brief Performs one cycle of sampling.
 */
void TTC_sample_cycle(void);

void TTC_timed_event_1_handler(void);
void TTC_timed_event_2_handler(void);
void TTC_timed_event_3_handler(void);
void TTC_timed_event_4_handler(void);
void TTC_timed_event_5_handler(void);
void TTC_timed_event_6_handler(void);
void TTC_timed_event_7_handler(void);
void TTC_timed_event_8_handler(void);
void TTC_timed_event_9_handler(void);
void TTC_timed_event_10_handler(void);

void TTC_timed_event_11_handler(void);
void TTC_timed_event_12_handler(void);
void TTC_timed_event_13_handler(void);
void TTC_timed_event_14_handler(void);
void TTC_timed_event_15_handler(void);
void TTC_timed_event_16_handler(void);
void TTC_timed_event_17_handler(void);
void TTC_timed_event_18_handler(void);
void TTC_timed_event_19_handler(void);
void TTC_timed_event_20_handler(void);

void save_buffer_to_SD(csv_t* csv);

void deployment_blinker(void);

void retraction_blinker(void);

/**
 * @brief Turns an array of booleans into a double which represents them all bitwise.
 * @param bool_array bool* pointer to the bool array
 * @param size int size of the bool array
 */
double crunch_flags(bool bool_array[], int size);

void write_sample_to_csv(csv_t* storage_buffer, sample_t* sample, int first_column_index);

void generate_next_filename(char* buffer, int buffer_size, const char* base_name);

#endif
