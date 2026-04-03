
#ifndef TT_CONTROLLER_H
#define TT_CONTROLLER_H

#include "flight_software/drivers/testbed_temperature_driver.h"
#include "flight_software/flight_software_types.h"
#include "simulation/real_time_engine.h"

/* all caps means peripheral to this scope
TIMESTAMP
CONTROLLER STATE
CONTROLLER ERRORS
driver state
driver errors
flags
data
*/
#define TTC_PERIPHERAL_COLUMNS 3
#define TTC_DRIVERS_UTILIZED 1
#define TTC_OUTPUT_FILE_NAME "Testbed Thermometer Data"
#define TTC_STORAGE_COLUMNS ( TTD_CSV_COLUMNS_COUNT + TTC_PERIPHERAL_COLUMNS )
#define TTC_DATA_BUFFER_SIZE ( TTC_STORAGE_COLUMNS * TTD_SAMPLES_PER_WRITE )

typedef enum TTC_errors_e TTC_errors_e;
enum TTC_errors_e {

    C_ERROR_ENVIRONMENT_FALLTHROUGH_E,
    C_ERROR_TIMED_EVENT_FALLTHROUGH_E,
    C_ERROR_DEPLOYMENT_WHILE_UNINITIALIZED_E,
    C_ERROR_UNPREPARED_TO_SAMPLE_E,

TTC_ERROR_COUNT
};

/**
 * @brief Entry point for the controller. Use this to link up the controller struct
 */
void setup(void);

/**
 * @brief Main looping function for the microcontroller
 * @note Wee haw!
 * @todo ADD A GIANT ERROR MESSAGE IN HERE
 */
void loop(void);

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

void deploy(void);

void retract(void);

/**
 * @brief Performs one cycle of sampling.
 */
void sample_cycle(void);

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

#endif
