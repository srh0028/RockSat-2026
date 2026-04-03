
#include "flight_software/controllers/testbed_temperature_controller.h"
#include "flight_software/drivers/testbed_temperature_driver.h"
#include "flight_software/flight_software_types.h"
#include <time.h>

environment_e environment = FLIGHT_E;
timed_event_handler TTC_event_handlers[ TIMED_EVENT_COUNT ] = {

    TTC_timed_event_1_handler,
    TTC_timed_event_2_handler,
    TTC_timed_event_3_handler,
    TTC_timed_event_4_handler,
    TTC_timed_event_5_handler,
    TTC_timed_event_6_handler,
    TTC_timed_event_7_handler,
    TTC_timed_event_8_handler,
    TTC_timed_event_9_handler,
    TTC_timed_event_10_handler,

    TTC_timed_event_11_handler,
    TTC_timed_event_12_handler,
    TTC_timed_event_13_handler,
    TTC_timed_event_14_handler,
    TTC_timed_event_15_handler,
    TTC_timed_event_16_handler,
    TTC_timed_event_17_handler,
    TTC_timed_event_18_handler,
    TTC_timed_event_19_handler,
    TTC_timed_event_20_handler
};
sample_t TTC_sample_buffer = {

    0.0,
    { 0.0 },
    0,
    0,
    0.0,
    0.0
};
double TTC_data_buffer[ TTC_DATA_BUFFER_SIZE ] = { 0.0 };
char* TTC_csv_column_names[ TTC_STORAGE_COLUMNS ] = {

    "timestamp",
    "controller state",
    "controller errors",
    "driver state",
    "driver errors",
    "flags",
    "data"
};
csv_t TTC_storage_buffer = {

    TTC_OUTPUT_FILE_NAME,
    0,
    TTC_STORAGE_COLUMNS,
    TTC_csv_column_names,
    false,
    &TTC_data_buffer[ 0 ],
    TTD_SAMPLES_PER_WRITE,
    0
};
instrument_t TT_instrument = {

    D_UNINITIALIZED_E,
    TTD_deploy_instrumentation,
    TTD_retract_instrumentation,
    TTD_sample,
    -1,
    false,
    &TTC_sample_buffer,
    &TTC_storage_buffer,
    TTC_PERIPHERAL_COLUMNS //starts printing in the CSV after the controller's peripheral columns
};
controller_t TT_controller = {

    NO_TIMED_EVENT_E,
    C_UNINITIALIZED_E,
    { NULL },
    NULL,
    false,
    NULL
};
bool errors[ TTC_ERROR_COUNT ] = { false };

void setup(void) {

    //Link up the function pointers into their respective data structures
    TTC_event_handlers[ 0 ] = timed_event_1_handler;
    TTC_event_handlers[ 1 ] = timed_event_2_handler;
    TTC_event_handlers[ 2 ] = timed_event_3_handler;
    TTC_event_handlers[ 3 ] = timed_event_4_handler;
    TTC_event_handlers[ 4 ] = timed_event_5_handler;
    TTC_event_handlers[ 5 ] = timed_event_6_handler;
    TTC_event_handlers[ 6 ] = timed_event_7_handler;
    TTC_event_handlers[ 7 ] = timed_event_8_handler;
    TTC_event_handlers[ 8 ] = timed_event_9_handler;
    TTC_event_handlers[ 9 ] = timed_event_10_handler;

    TTC_event_handlers[ 10 ] = timed_event_11_handler;
    TTC_event_handlers[ 11 ] = timed_event_12_handler;
    TTC_event_handlers[ 12 ] = timed_event_13_handler;
    TTC_event_handlers[ 13 ] = timed_event_14_handler;
    TTC_event_handlers[ 14 ] = timed_event_15_handler;
    TTC_event_handlers[ 15 ] = timed_event_16_handler;
    TTC_event_handlers[ 16 ] = timed_event_17_handler;
    TTC_event_handlers[ 17 ] = timed_event_18_handler;
    TTC_event_handlers[ 18 ] = timed_event_19_handler;
    TTC_event_handlers[ 19 ] = timed_event_20_handler;

    TT_controller.timed_event_handlers = TTC_event_handlers;

    TT_controller.loop = loop;

    //formally initialize the controller
    controller_state_e status = initialize();

    //presume no errors have occured yet
    for ( int i = 0; i < TTC_ERROR_COUNT; i ++ ) errors[ i ] = false;
}

void loop(void) {

    //initialize if necessary (THIS SHOULD NEVER HAPPEN BUT JUST IN CASE?)
    if ( TT_controller.state == C_UNINITIALIZED_E ) initialize();

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
            TT_controller.ready_for_next_timed_event = true;
            break;

        //allow only valid transitions
        case 1:
            if ( TT_controller.ready_for_next_timed_event == false ) break;
            TT_controller.ready_for_next_timed_event = false;
            if ( TT_controller.timed_event == NO_TIMED_EVENT_E ) TT_controller.timed_event = TIMED_EVENT_1_E;
            else TT_controller.timed_event++;
            break;

        default:
            errors[ C_ERROR_TIMED_EVENT_FALLTHROUGH_E ] = true;
            return;
    }

    //finally, perform the actions slated for the timed event we are in
    if ( TT_controller.timed_event != NO_TIMED_EVENT_E ) {

        TT_controller.timed_event_handlers[ TT_controller.timed_event ]();
    }
}

controller_state_e TTC_initialize(void) {

    //Initialize a driver on top of the statically allocated instrument
    initialize_driver( &TT_instrument, 
                        0 /*No deployment*/, 
                        &TTC_data_buffer, 
                        &TTC_storage_buffer );

    //plug all my instruments into the array
    TT_controller.instruments[ 0 ] = &TT_instrument;

    //update and return
    TT_controller.state = C_UNDEPLOYED_E;
    return C_UNDEPLOYED_E;
}

int TTC_read_in_sim_timed_event(void) {

    if ( simulation.timed_event_pin > 0 ) return 1;
    return 0;
}

int TTC_read_in_flight_timed_event(void) {

    /*//RETURN 1 IF THE BUTTON IS PUSHED*/
    return 0;
}

void sample_cycle(void) {

    //guard condition
    if ( TT_controller.state != C_READY_E ) errors[ C_ERROR_UNPREPARED_TO_SAMPLE_E ] = true;

    //Begin sample cycle
    TT_controller.state = C_SAMPLING_E;
    instrument_t* instrument;
    csv_t* storage_buffer = &TTC_storage_buffer;
    int where = storage_buffer->cursor * storage_buffer->columns_int;

    double clock_dbl = (double) clock() * 1000 / CLOCKS_PER_SEC;    
    storage_buffer->data_ptr[ where ] = clock_dbl;
    double state_dbl = (double) TT_controller.state;
    storage_buffer->data_ptr[ where + 1 ] = state_dbl;
    double error_flags_dbl = crunch_flags( errors, TTC_ERROR_COUNT );
    storage_buffer->data_ptr[ where + 2 ] = error_flags_dbl;
    for ( int driver = 0; driver < TTC_DRIVERS_UTILIZED; driver++ ) {

        //grab each driver
        instrument = TT_controller.instruments[ driver ];
        // if ( instrument->driver_state != D_READY_E ) continue; 
        if ( storage_buffer->cursor >= TTD_SAMPLES_PER_WRITE ) {

            //save that driver's storage buffer to memory if it's time and reset that buffer
            save_buffer_to_sim_sd( storage_buffer );
            storage_buffer->cursor = -1;
            memset( &TTC_data_buffer[ 0 ], 0.0, sizeof( TTC_data_buffer ) );
        }
        instrument->sample();
    }

    //reset controller state
    storage_buffer->cursor ++;
    TT_controller.state = C_READY_E;
}

void deploy(void) {

    //guard condition
    if ( TT_controller.state == C_UNINITIALIZED_E ) {

        errors[ C_ERROR_DEPLOYMENT_WHILE_UNINITIALIZED_E ] = true;
        initialize();
    }

    //Ensure all instruments are deploying. Do not start sampling before complete deployment
    instrument_t* instrument;
    bool any_remain_to_deploy = false;
    for ( int driver = 0; driver < TTC_DRIVERS_UTILIZED; driver++ ) {

        instrument = TT_controller.instruments[ driver ];
        if ( instrument->deployed == false ) {

            TT_controller.state = C_DEPLOYING_E;
            instrument->deploy();
            any_remain_to_deploy = true;
        }
    }

    //you shouldn't need me to tell you what this does
    if ( any_remain_to_deploy == false ) TT_controller.state = C_READY_E;
}

void retract(void) {

    //guard condition
    if ( TT_controller.state != C_RETRACTING_E ) {

        if ( TT_controller.state != C_READY_E ) return;
    }

    //ensure all instruments are retracting
    instrument_t* instrument;
    bool any_motors_left_to_retract = false;
    for ( int driver = 0; driver < TTC_DRIVERS_UTILIZED; driver++ ) {

        instrument = TT_controller.instruments[ driver ];
        if ( instrument->deployed == true ) {

            TT_controller.state = C_RETRACTING_E;
            any_motors_left_to_retract = true;
            instrument->retract();
        }
    }

    //update controller state
    if ( any_motors_left_to_retract == false ) TT_controller.state = C_UNDEPLOYED_E;
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
