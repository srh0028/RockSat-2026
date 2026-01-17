
#ifndef RTE_H
#define RTE_H

typedef enum {

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

typedef enum {

    WAITING_EV,
    SIMULATING_EV,
    FINISHED_EV,

SIMULATOR_STATUS_COUNT
} simulator_status_e;

/**
 * @brief Resets the simulator's state to prepare for the next simulation.
 */
int reset_simulation( csv_t* flight_profile_ptr );

/**
 * @brief Begins simulation.
 */
void begin_simulation(void);

/**
 * @brief Pushes a timed event to the controllers during simulation.
 */
void push_timed_event( timed_event_e event );

/**
 * @brief Execute one tick of simulation.
 */
void simulation_tick(void);

/**
 * @brief Cleans up after a simulation.
 */
void conclude_simulation(void);

#endif
