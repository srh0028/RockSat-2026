
#ifndef GENERIC_CONTROLLER_H
#define GENERIC_CONTROLLER_H

#include "flight_software/drivers/generic_driver.h"
#include "flight_software/flight_software_types.h"
#include "simulation/real_time_engine.h"

#define DRIVERS_UTILIZED_GENERIC 1

/**
 * @brief Entry point for the controller. Use this to link up the controller struct
 */
void generic_controller_setup(void);

/**
 * @brief Main looping function for the microcontroller
 * @note Wee haw!
 * @todo ADD A GIANT ERROR MESSAGE IN HERE
 */
void generic_controller_loop(void);

/**
 * @brief Initialize a controller with the given number of instruments.
 * @returns controller state after initialization
 */
controller_state_e initialize(void);

/**
 * @brief Initializes storage for the given environment.
 * @retval -1: failure
 * @retval 1: success
 */
int initialize_storage(void);

/**
 * @brief Reads in a timed event from the simulation.
 * @retval 1 if a timed event is active
 * @retval 0 if a timed event is not active
 */
int read_in_sim_timed_event(void);

/**
 * @brief Reads in a timed event in flight.
 * @retval 0 TE pin = logic low
 * @retval 1 TE pin = logic high
 */
int read_in_flight_timed_event(void);

void deploy(void);

void retract(void);

/**
 * @brief Performs one cycle of sampling.
 */
void sample_cycle(void);

void timed_event_1_handler(void);
void timed_event_2_handler(void);
void timed_event_3_handler(void);
void timed_event_4_handler(void);
void timed_event_5_handler(void);
void timed_event_6_handler(void);
void timed_event_7_handler(void);
void timed_event_8_handler(void);
void timed_event_9_handler(void);
void timed_event_10_handler(void);

void timed_event_11_handler(void);
void timed_event_12_handler(void);
void timed_event_13_handler(void);
void timed_event_14_handler(void);
void timed_event_15_handler(void);
void timed_event_16_handler(void);
void timed_event_17_handler(void);
void timed_event_18_handler(void);
void timed_event_19_handler(void);
void timed_event_20_handler(void);

#endif
