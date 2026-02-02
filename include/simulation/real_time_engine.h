
#ifndef RTE_H
#define RTE_H

#include "core/common/storage.h"
#include <stdbool.h>
typedef enum
{

    TIMED_EVENT_1_EV,
    TIMED_EVENT_2_EV,
    TIMED_EVENT_3_EV,
    TIMED_EVENT_4_EV,
    TIMED_EVENT_5_EV,
    TIMED_EVENT_6_EV,
    TIMED_EVENT_7_EV,
    TIMED_EVENT_8_EV,
    TIMED_EVENT_9_EV,
    TIMED_EVENT_10_EV,
    TIMED_EVENT_11_EV,
    TIMED_EVENT_12_EV,
    TIMED_EVENT_13_EV,
    TIMED_EVENT_14_EV,
    TIMED_EVENT_15_EV,
    TIMED_EVENT_16_EV,
    TIMED_EVENT_17_EV,
    TIMED_EVENT_18_EV,
    TIMED_EVENT_19_EV,
    TIMED_EVENT_20_EV,

    TIMED_EVENT_COUNT
} timed_event_e;

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
typedef struct simulation_t simulation_t;
struct simulation_t
{

    csv_t *flight_profile_ptr;
    simulator_status_e status_e;
    timed_event_e current_timed_event;
    int current_tick_int;
    double current_altitude_dbl;
};

extern simulation_t simulation;

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
void push_timed_event(timed_event_e event_e);

/**
 * @brief Cleans up after a simulation.
 */
void conclude_simulation(void);

void sim_snapshot(void);

#endif