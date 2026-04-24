
#ifndef LANGMUIR_TYPES_H
#define LANGMUIR_TYPES_H

#include <stdbool.h>
#include <time.h>
#include "langmuir_driver.h"

#define DOUBLES_PER_SAMPLE 5

typedef struct sample_t sample_t;
typedef struct instrument_t instrument_t;
typedef struct controller_t controller_t;
typedef int ( *read_timed_event_pin )(void);
typedef void ( *deployment_function )(void);
typedef void ( *retraction_function )(void);
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
    D_UNDEPLOYED_E,
    D_DEPLOYING_E,
    D_READY_E,
    D_SAMPLING_E,
    D_ERROR_E,

DRIVER_STATE_COUNT
};

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

enum timed_event_e {

    TIMED_EVENT_1_E,
    TIMED_EVENT_2_E,

TIMED_EVENT_COUNT,
NO_TIMED_EVENT_E
};

enum instrument_e {

    INSTRUMENT_0_E,

INSTRUMENT_COUNT,
MAX_INSTRUMENT_COUNT = 8
};

enum environment_e {

    FLIGHT_E,
    SIMULATION_E,

ENVIRONMENT_COUNT
};

struct sample_t {

    double samples[ DOUBLES_PER_SAMPLE ];
    double driver_error_flags;
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
    bool errors[ LPD_ERROR_COUNT ];
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
