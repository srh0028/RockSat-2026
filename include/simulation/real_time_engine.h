
#ifndef RTE_H
#define RTE_H

#include "storage.h"
#include "flight_software/flight_software_types.h"
#include "simulation/simulation_config.h"
#include <stdbool.h>

typedef struct simulation_t simulation_t;

extern simulation_t simulation;
extern controller_t generic_controller;

typedef enum controllers_e {

    CONTROLLER_1_E,
    CONTROLLER_2_E,
    CONTROLLER_3_E,
    CONTROLLER_4_E,
    CONTROLLER_5_E,
    CONTROLLER_6_E,
    CONTROLLER_7_E,
    CONTROLLER_8_E,

CONTROLLER_COUNT
} controllers_e;

typedef enum
{

    WAITING_EV,
    SIMULATING_EV,
    FINISHED_EV,

    SIMULATOR_STATUS_COUNT
} simulator_status_e;

/**
 * @brief Represents a simulation.
 */
struct simulation_t {

    csv_t *flight_profile_ptr;
    simulator_status_e status_e;
    timed_event_e current_timed_event;
    int timed_event_pin;
    int current_tick_int;
    double current_altitude_dbl;
    controller_t* controllers[ ACTIVE_CONTROLLERS_COUNT ];
};

/**
 * @brief Resets the simulator's state to prepare for the next simulation.
 * @param flight_profile_ptr Which flight profile to set up to simulate.
 * @retval -1: Simulation already in progress
 * @retval -2: Invalid flight profile
 * @retval 1: Ready to simulate
 */
int reset_simulation(csv_t *flight_profile_ptr);

/**
 * @brief Runs the simulation.
 */
void simulate(void);

/**
 * @brief Pushes a timed event to the controllers during simulation.
 */
void push_timed_event(void);

/**
 * @brief Cleans up after a simulation.
 */
void conclude_simulation(void);

void sim_snapshot(void);

#endif