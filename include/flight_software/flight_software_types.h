
#ifndef FLIGHT_SOFTWARE_TYPES_H
#define FLIGHT_SOFTWARE_TYPES_H

#include "storage.h"
#include <stdbool.h>
#include <time.h>

#define MAX_DOUBLES_PER_SAMPLE 16
#define MAX_DRIVERS_PER_CONTROLLER 8
#define ENCODER_UNITS_PER_INCH 385.75
#define MAX_IMAGINABLE_BOOM_EXTENSION_INCHES 120
#define MAX_ERROR_FLAGS 8

typedef enum controller_state_e controller_state_e;
typedef enum driver_state_e driver_state_e;
typedef enum timed_event_e timed_event_e;
typedef enum instrument_e instrument_e;
typedef enum environment_e environment_e;
typedef struct sample_t sample_t;
typedef struct instrument_t instrument_t;
typedef struct controller_t controller_t;
typedef int ( *read_timed_event_pin )(void);
typedef int ( *deployment_function )(void);
typedef int ( *retraction_function )(void);
typedef void ( *sample_function )(void);
typedef void ( *controller_loop )(void);
typedef void ( *timed_event_handler )(void);
typedef void ( *controller_setup )(void);
typedef bool ( *data_flag_condition )( sample_t* sample );

enum controller_state_e {

    C_UNINITIALIZED_E,
    C_UNDEPLOYED_E,
    C_DEPLOYING_E,
    C_READY_E,
    C_SAMPLING_E,
    C_RETRACTING_E,

CONTROLLER_STATE_COUNT
};

enum driver_state_e {

    D_UNINITIALIZED_E,
    D_DEPLOYMENT_E,
    D_READY_E,
    D_SAMPLING_E,
    D_ERROR_E,

DRIVER_STATE_COUNT
};

enum timed_event_e {

    TIMED_EVENT_1_E,
    TIMED_EVENT_2_E,
    TIMED_EVENT_3_E,
    TIMED_EVENT_4_E,
    TIMED_EVENT_5_E,
    TIMED_EVENT_6_E,
    TIMED_EVENT_7_E,
    TIMED_EVENT_8_E,
    TIMED_EVENT_9_E,
    TIMED_EVENT_10_E,

    TIMED_EVENT_11_E,
    TIMED_EVENT_12_E,
    TIMED_EVENT_13_E,
    TIMED_EVENT_14_E,
    TIMED_EVENT_15_E,
    TIMED_EVENT_16_E,
    TIMED_EVENT_17_E,
    TIMED_EVENT_18_E,
    TIMED_EVENT_19_E,
    TIMED_EVENT_20_E,

TIMED_EVENT_COUNT,
NO_TIMED_EVENT_E
};

enum instrument_e {

    INSTRUMENT_0_E,
    INSTRUMENT_1_E,
    INSTRUMENT_2_E,
    INSTRUMENT_3_E,
    INSTRUMENT_4_E,
    INSTRUMENT_5_E,
    INSTRUMENT_6_E,
    INSTRUMENT_7_E,
    INSTRUMENT_8_E,
    INSTRUMENT_9_E,

    INSTRUMENT_10_E,
    INSTRUMENT_11_E,
    INSTRUMENT_12_E,
    INSTRUMENT_13_E,
    INSTRUMENT_14_E,
    INSTRUMENT_15_E,
    INSTRUMENT_16_E,
    INSTRUMENT_17_E,
    INSTRUMENT_18_E,
    INSTRUMENT_19_E,

    INSTRUMENT_GENERIC_E,

INSTRUMENT_COUNT,
MAX_INSTRUMENT_COUNT = 8
};

enum environment_e {

    FLIGHT_E,
    SIMULATION_E,

ENVIRONMENT_COUNT
};

struct sample_t {

    double samples[ MAX_DOUBLES_PER_SAMPLE ];
    int sample_double_count;
    unsigned char data_flags;
    double driver_state;
    double driver_error_flags
};

/**
 * @brief Represents a .csv in memory.
 */
typedef struct csv_t csv_t;
struct csv_t {

    char* file_name_ptr;
    int rows_int;
    int columns_int;
    char** column_names;
    bool headers_printed;
    double* data_ptr;
    int max_rows;
    int cursor;
};

struct instrument_t {

    driver_state_e driver_state;
    deployment_function deploy;
    retraction_function retract;
    sample_function sample;
    int target_deployment_units;
    bool deployed;
    sample_t* sample_buffer;
    csv_t* storage_buffer;
    int first_index_in_csv;
};

struct controller_t {

    timed_event_e timed_event;
    controller_state_e state;
    instrument_t* instruments[ MAX_INSTRUMENT_COUNT ];
    timed_event_handler* timed_event_handlers;
    bool ready_for_next_timed_event;
    controller_loop loop;
};

#endif
