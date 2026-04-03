
#include "flight_software/controllers/testbed_temperature_controller.h"
#include "flight_software/drivers/testbed_temperature_driver.h"
#include "flight_software/flight_software_types.h"
#include <time.h>

environment_e environment = SIMULATION_E;
timed_event_handler generic_event_handlers[ TIMED_EVENT_COUNT ] = {

    timed_event_1_handler,
    timed_event_2_handler,
    timed_event_3_handler,
    timed_event_4_handler,
    timed_event_5_handler,
    timed_event_6_handler,
    timed_event_7_handler,
    timed_event_8_handler,
    timed_event_9_handler,
    timed_event_10_handler,

    timed_event_11_handler,
    timed_event_12_handler,
    timed_event_13_handler,
    timed_event_14_handler,
    timed_event_15_handler,
    timed_event_16_handler,
    timed_event_17_handler,
    timed_event_18_handler,
    timed_event_19_handler,
    timed_event_20_handler
};
sample_t generic_sample_buffer = {

    0.0,
    { 0.0 },
    0,
    0,
    0.0,
    0.0
};
double generic_csv_data_buffer[ GENERIC_DATA_BUFFER_SIZE ] = { 0.0 };
char* csv_column_names[ GENERIC_STORAGE_COLUMNS ] = {

    "timestamp",
    "controller state",
    "controller errors",
    "driver state",
    "driver errors",
    "flags",
    "data"
};
csv_t generic_storage_buffer = {

    GENERIC_OUTPUT_FILE_NAME,
    0,
    GENERIC_STORAGE_COLUMNS,
    csv_column_names,
    false,
    &generic_csv_data_buffer[ 0 ],
    GENERIC_SAMPLES_PER_WRITE,
    0
};
instrument_t generic_instrument = {

    D_UNINITIALIZED_E,
    generic_deploy_instrumentation,
    generic_retract_instrumentation,
    generic_sample,
    -1,
    false,
    &generic_sample_buffer,
    &generic_storage_buffer,
    GC_PERIPHERAL_COLUMNS //starts printing in the CSV after the controller's peripheral columns
};
controller_t generic_controller = {

    NO_TIMED_EVENT_E,
    C_UNINITIALIZED_E,
    { NULL },
    NULL,
    false,
    NULL
};
bool errors[ GENERIC_CONTROLLER_ERROR_COUNT ] = { false };

void generic_controller_setup(void) {

    //Link up the function pointers into their respective data structures
    generic_event_handlers[ 0 ] = timed_event_1_handler;
    generic_event_handlers[ 1 ] = timed_event_2_handler;
    generic_event_handlers[ 2 ] = timed_event_3_handler;
    generic_event_handlers[ 3 ] = timed_event_4_handler;
    generic_event_handlers[ 4 ] = timed_event_5_handler;
    generic_event_handlers[ 5 ] = timed_event_6_handler;
    generic_event_handlers[ 6 ] = timed_event_7_handler;
    generic_event_handlers[ 7 ] = timed_event_8_handler;
    generic_event_handlers[ 8 ] = timed_event_9_handler;
    generic_event_handlers[ 9 ] = timed_event_10_handler;

    generic_event_handlers[ 10 ] = timed_event_11_handler;
    generic_event_handlers[ 11 ] = timed_event_12_handler;
    generic_event_handlers[ 12 ] = timed_event_13_handler;
    generic_event_handlers[ 13 ] = timed_event_14_handler;
    generic_event_handlers[ 14 ] = timed_event_15_handler;
    generic_event_handlers[ 15 ] = timed_event_16_handler;
    generic_event_handlers[ 16 ] = timed_event_17_handler;
    generic_event_handlers[ 17 ] = timed_event_18_handler;
    generic_event_handlers[ 18 ] = timed_event_19_handler;
    generic_event_handlers[ 19 ] = timed_event_20_handler;

    generic_controller.timed_event_handlers = generic_event_handlers;

    generic_controller.loop = generic_controller_loop;

    //formally initialize the controller
    controller_state_e status = initialize();

    //presume no errors have occured yet
    for ( int i = 0; i < GENERIC_CONTROLLER_ERROR_COUNT; i ++ ) errors[ i ] = false;
}

void generic_controller_loop(void) {

    //initialize if necessary (THIS SHOULD NEVER HAPPEN BUT JUST IN CASE?)
    if ( generic_controller.state == C_UNINITIALIZED_E ) initialize();

    //controller must be initialized by this point. check for timed events
    int status = -1;
    switch ( environment ) {

        case SIMULATION_E:
            status = read_in_sim_timed_event();
            break;

        case FLIGHT_E:
            status = read_in_flight_timed_event();
            break;

        default:
            errors[ C_ERROR_ENVIRONMENT_FALLTHROUGH_E ] = true;
            return;
    }

    //manage TE transitions
    switch ( status ) {

        case 0:
            generic_controller.ready_for_next_timed_event = true;
            break;

        //allow only valid transitions
        case 1:
            if ( generic_controller.ready_for_next_timed_event == false ) break;
            generic_controller.ready_for_next_timed_event = false;
            if ( generic_controller.timed_event == NO_TIMED_EVENT_E ) generic_controller.timed_event = TIMED_EVENT_1_E;
            else generic_controller.timed_event++;
            break;

        default:
            errors[ C_ERROR_TIMED_EVENT_FALLTHROUGH_E ] = true;
            return;
    }

    //finally, perform the actions slated for the timed event we are in
    if ( generic_controller.timed_event != NO_TIMED_EVENT_E ) {

        generic_controller.timed_event_handlers[ generic_controller.timed_event ]();
    }
}

controller_state_e initialize(void) {

    //Initialize a driver on top of the statically allocated instrument
    initialize_driver( &generic_instrument, 
                        1 /*Only try to deploy 1 inch*/, 
                        &generic_sample_buffer, 
                        &generic_storage_buffer );

    //plug all my instruments into the array
    generic_controller.instruments[ 0 ] = &generic_instrument;

    //update and return
    generic_controller.state = C_UNDEPLOYED_E;
    return C_UNDEPLOYED_E;
}

int read_in_sim_timed_event(void) {

    if ( simulation.timed_event_pin > 0 ) return 1;
    return 0;
}

int read_in_flight_timed_event(void) {

    return 0;
}

void sample_cycle(void) {

    // printf( "%s\n", "sample_cycle()" );

    //guard condition
    if ( generic_controller.state != C_READY_E ) errors[ C_ERROR_UNPREPARED_TO_SAMPLE_E ] = true;

    //Begin sample cycle
    generic_controller.state = C_SAMPLING_E;
    instrument_t* instrument;
    csv_t* storage_buffer = &generic_storage_buffer;
    int where = storage_buffer->cursor * storage_buffer->columns_int;

    // printf( "%s%d\n", "where index: ", where );
    // printf( "%s%d\n", "cursor: ", storage_buffer->cursor );


    double clock_dbl = (double) clock() * 1000 / CLOCKS_PER_SEC;
    
    // printf( "%s%f\n", "clock: ", clock_dbl );
    
    storage_buffer->data_ptr[ where ] = clock_dbl;


    double state_dbl = (double) generic_controller.state;

    // printf( "%s%f\n", "state: ", state_dbl );

    storage_buffer->data_ptr[ where + 1 ] = state_dbl;
    

    double error_flags_dbl = crunch_flags( errors, GENERIC_CONTROLLER_ERROR_COUNT );
    
    // printf( "%s%f\n", "flags: ", error_flags_dbl );

    storage_buffer->data_ptr[ where + 2 ] = error_flags_dbl;


    for ( int driver = 0; driver < DRIVERS_UTILIZED_GENERIC; driver++ ) {

        //grab each driver
        instrument = generic_controller.instruments[ driver ];
        // if ( instrument->driver_state != D_READY_E ) continue; 
        if ( storage_buffer->cursor >= GENERIC_SAMPLES_PER_WRITE ) {

            //save that driver's storage buffer to memory if it's time and reset that buffer
            save_buffer_to_sim_sd( storage_buffer );
            storage_buffer->cursor = -1;
            memset( &generic_csv_data_buffer[ 0 ], 0.0, sizeof( generic_csv_data_buffer ) );
        }
        instrument->sample();
    }

    //reset controller state
    storage_buffer->cursor ++;
    generic_controller.state = C_READY_E;
}

void deploy(void) {

    //guard condition
    if ( generic_controller.state == C_UNINITIALIZED_E ) {

        errors[ C_ERROR_DEPLOYMENT_WHILE_UNINITIALIZED_E ] = true;
        initialize();
    }

    //Ensure all instruments are deploying. Do not start sampling before complete deployment
    instrument_t* instrument;
    bool any_remain_to_deploy = false;
    for ( int driver = 0; driver < DRIVERS_UTILIZED_GENERIC; driver++ ) {

        instrument = generic_controller.instruments[ driver ];
        if ( instrument->deployed == false ) {

            generic_controller.state = C_DEPLOYING_E;
            instrument->deploy();
            any_remain_to_deploy = true;
        }
    }

    //you shouldn't need me to tell you what this does
    if ( any_remain_to_deploy == false ) generic_controller.state = C_READY_E;
}

void retract(void) {

    //guard condition
    if ( generic_controller.state != C_RETRACTING_E ) {

        if ( generic_controller.state != C_READY_E ) return;
    }

    //ensure all instruments are retracting
    instrument_t* instrument;
    bool any_motors_left_to_retract = false;
    for ( int driver = 0; driver < DRIVERS_UTILIZED_GENERIC; driver++ ) {

        instrument = generic_controller.instruments[ driver ];
        if ( instrument->deployed == true ) {

            generic_controller.state = C_RETRACTING_E;
            any_motors_left_to_retract = true;
            instrument->retract();
        }
    }

    //update controller state
    if ( any_motors_left_to_retract == false ) generic_controller.state = C_UNDEPLOYED_E;
}

//DEPLOY IF NOT DEPLOYED, SAMPLE IF DEPLOYED
void timed_event_1_handler(void) {

    deploy();
    sample_cycle();
}

//SAMPLE IF DEPLOYED
void timed_event_2_handler(void) {
    
    sample_cycle();
}

//SAMPLE IF DEPLOYED
void timed_event_3_handler(void) {

    sample_cycle();
}

//RETRACT
void timed_event_4_handler(void) {

    retract();
}

//SAMPLE IF DEPLOYED (SHOULD FAIL)
void timed_event_5_handler(void) {

    sample_cycle();
}

//STUBS
void timed_event_6_handler(void) { return; }
void timed_event_7_handler(void) { return; }
void timed_event_8_handler(void) { return; }
void timed_event_9_handler(void) { return; }
void timed_event_10_handler(void) { return; }

void timed_event_11_handler(void) { return; }
void timed_event_12_handler(void) { return; }
void timed_event_13_handler(void) { return; }
void timed_event_14_handler(void) { return; }
void timed_event_15_handler(void) { return; }
void timed_event_16_handler(void) { return; }
void timed_event_17_handler(void) { return; }
void timed_event_18_handler(void) { return; }
void timed_event_19_handler(void) { return; }
void timed_event_20_handler(void) { return; }
