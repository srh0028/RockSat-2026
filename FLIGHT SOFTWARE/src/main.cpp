
#include "testbed_temperature_controller.h"
#include "testbed_temperature_driver.h"
#include "testbed_types.h"
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>

environment_e TTC_environment = FLIGHT_E;
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

    "millis",
    "delta millis",
    "mission seconds",
    "controller state",
    "controller errors",
    "driver state",
    "driver errors",
    "flags",
    "raw resistance",
    "converted temperature"
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
bool TTC_errors[ TTC_ERROR_COUNT ] = { false };

// Deployment blinker globals
unsigned long deploy_current_time = 0;
unsigned long deploy_last_call_time = 0;
unsigned long deploy_delta_time = 0;
unsigned long deploy_accumulated_time = 0;
bool deploy_blinker_on = false;

// Retraction blinker globals
unsigned long retract_current_time = 0;
unsigned long retract_last_call_time = 0;
unsigned long retract_delta_time = 0;
unsigned long retract_accumulated_time = 0;
bool retract_blinker_on = false;

void setup(void) {

    //Link up the function pointers into their respective data structures
    TTC_event_handlers[ 0 ] = TTC_timed_event_1_handler;
    TTC_event_handlers[ 1 ] = TTC_timed_event_2_handler;
    TTC_event_handlers[ 2 ] = TTC_timed_event_3_handler;
    TTC_event_handlers[ 3 ] = TTC_timed_event_4_handler;
    TTC_event_handlers[ 4 ] = TTC_timed_event_5_handler;
    TTC_event_handlers[ 5 ] = TTC_timed_event_6_handler;
    TTC_event_handlers[ 6 ] = TTC_timed_event_7_handler;
    TTC_event_handlers[ 7 ] = TTC_timed_event_8_handler;
    TTC_event_handlers[ 8 ] = TTC_timed_event_9_handler;
    TTC_event_handlers[ 9 ] = TTC_timed_event_10_handler;

    TTC_event_handlers[ 10 ] = TTC_timed_event_11_handler;
    TTC_event_handlers[ 11 ] = TTC_timed_event_12_handler;
    TTC_event_handlers[ 12 ] = TTC_timed_event_13_handler;
    TTC_event_handlers[ 13 ] = TTC_timed_event_14_handler;
    TTC_event_handlers[ 14 ] = TTC_timed_event_15_handler;
    TTC_event_handlers[ 15 ] = TTC_timed_event_16_handler;
    TTC_event_handlers[ 16 ] = TTC_timed_event_17_handler;
    TTC_event_handlers[ 17 ] = TTC_timed_event_18_handler;
    TTC_event_handlers[ 18 ] = TTC_timed_event_19_handler;
    TTC_event_handlers[ 19 ] = TTC_timed_event_20_handler;

    TT_controller.timed_event_handlers = TTC_event_handlers;

    TT_controller.loop = loop;

    //formally initialize the controller
    controller_state_e status = TTC_initialize();
    deploy_current_time = 0;
    deploy_last_call_time = 0;
    deploy_delta_time = 0;
    deploy_accumulated_time = 0;

    retract_current_time = 0;
    retract_last_call_time = 0;
    retract_delta_time = 0;
    retract_accumulated_time = 0;
    deploy_blinker_on = false;
    retract_blinker_on = false;

    //presume no errors have occured yet
    for ( int i = 0; i < TTC_ERROR_COUNT; i ++ ) TTC_errors[ i ] = false;
    
    //initialize UI
    pinMode( LED_PIN, OUTPUT );
    
    //initialize sd pins
    SPI.setRX(SD_MISO_PIN);
    SPI.setTX(SD_MOSI_PIN);
    SPI.setSCK(SD_SCK_PIN);
    SPI.begin();

    if ( !SD.begin(SD_CS_PIN) ) {

        //init failed
        digitalWrite( LED_PIN, HIGH );
        delay(5000);
    }
}

void loop(void) {

    //initialize if necessary (THIS SHOULD NEVER HAPPEN BUT JUST IN CASE?)
    if ( TT_controller.state == C_UNINITIALIZED_E ) TTC_initialize();

    //controller must be initialized by this point. check for timed events
    int status = -1;
    switch ( TTC_environment ) {

        case SIMULATION_E:
            status = TTC_read_in_sim_timed_event();
            break;

        case FLIGHT_E:
            status = TTC_read_in_flight_timed_event();
            break;

        default:
            TTC_errors[ C_ERROR_ENVIRONMENT_FALLTHROUGH_E ] = true;
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
            else TT_controller.timed_event = (timed_event_e)(TT_controller.timed_event + 1);
            break;

        default:
            TTC_errors[ C_ERROR_TIMED_EVENT_FALLTHROUGH_E ] = true;
            return;
    }

    //finally, perform the actions slated for the timed event we are in
    if ( TT_controller.timed_event != NO_TIMED_EVENT_E ) {

        TT_controller.timed_event_handlers[ TT_controller.timed_event ]();
    }
}

controller_state_e TTC_initialize(void) {

    //Initialize a driver on top of the statically allocated instrument
    TTD_initialize_driver( &TT_instrument, 
                        1 /*No deployment*/, 
                        &TTC_sample_buffer, 
                        &TTC_storage_buffer );

    //plug all my instruments into the array
    TT_controller.instruments[ 0 ] = &TT_instrument;

    //update and return
    TT_controller.state = C_UNDEPLOYED_E;
    return C_UNDEPLOYED_E;
}

int TTC_read_in_sim_timed_event(void) {

    // if ( simulation.timed_event_pin > 0 ) return 1;
    return 0;
}

int TTC_read_in_flight_timed_event(void) {

    if ( BOOTSEL ) return 1;
    return 0;
}

void TTC_sample_cycle(void) {

    //guard condition
    if ( TT_controller.state != C_READY_E ) TTC_errors[ C_ERROR_UNPREPARED_TO_SAMPLE_E ] = true;
    else TTC_errors[ C_ERROR_UNPREPARED_TO_SAMPLE_E ] = false;

    //Begin sample cycle
    TT_controller.state = C_SAMPLING_E;
    instrument_t* instrument;
    csv_t* storage_buffer = &TTC_storage_buffer;
    int where = storage_buffer->cursor * storage_buffer->columns_int;

    //Record timestamps
    static double previous_time = 0;
    double clock_dbl = (double) millis();
    double delta_time = clock_dbl - previous_time;
    previous_time = clock_dbl;
    double mission_seconds = clock_dbl / 1000;
    storage_buffer->data_ptr[ where ] = clock_dbl;
    storage_buffer->data_ptr[ where + 1 ] = delta_time;
    storage_buffer->data_ptr[ where + 2 ] = mission_seconds;

    //Record controller state and flags
    double state_dbl = (double) TT_controller.state;
    storage_buffer->data_ptr[ where + 3 ] = state_dbl;
    double error_flags_dbl = crunch_flags( TTC_errors, TTC_ERROR_COUNT );
    storage_buffer->data_ptr[ where + 4 ] = error_flags_dbl;
    for ( int driver = 0; driver < TTC_DRIVERS_UTILIZED; driver++ ) {

        //grab each driver
        instrument = TT_controller.instruments[ driver ];
        if ( storage_buffer->cursor >= TTD_SAMPLES_PER_WRITE ) {

            //save that driver's storage buffer to memory if it's time and reset that buffer
            save_buffer_to_SD( storage_buffer );
            storage_buffer->cursor = -1;
            memset( &TTC_data_buffer[ 0 ], 0.0, sizeof( TTC_data_buffer ) );
        }
        instrument->sample();
    }

    //reset controller state
    storage_buffer->cursor ++;
    TT_controller.state = C_READY_E;
}

void TTC_deploy(void) {

    //guard condition
    if ( TT_controller.state == C_UNINITIALIZED_E ) {

        TTC_errors[ C_ERROR_DEPLOYMENT_WHILE_UNINITIALIZED_E ] = true;
        TTC_initialize();
    }

    //Ensure all instruments attempt to deploy
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

    //operate the blinker
    deployment_blinker();

    //you shouldn't need me to tell you what this does
    if ( any_remain_to_deploy == false ) {

        TT_controller.state = C_READY_E;
        digitalWrite( LED_PIN, LOW ); //turn off the lights when you leave the room
    } 
}

void TTC_retract(void) {

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

    //operate the blinker
    retraction_blinker();

    //update controller state
    if ( any_motors_left_to_retract == false ) {

        TT_controller.state = C_UNDEPLOYED_E;
        digitalWrite( LED_PIN, LOW ); //turn off the lights when you leave the room
    } 
}

//DEPLOY, SAMPLE
void TTC_timed_event_1_handler(void) {

    TTC_deploy();
    TTC_sample_cycle();
}

//SAMPLE
void TTC_timed_event_2_handler(void) {
    
    TTC_sample_cycle();
}

//RETRACT, SAMPLE
void TTC_timed_event_3_handler(void) {

    TTC_retract();
}

//SAMPLE
void TTC_timed_event_4_handler(void) {

    TTC_sample_cycle();
}

//STUBS
void TTC_timed_event_5_handler(void) { return; }
void TTC_timed_event_6_handler(void) { return; };
void TTC_timed_event_7_handler(void) { return; };
void TTC_timed_event_8_handler(void) { return; };
void TTC_timed_event_9_handler(void) { return; };
void TTC_timed_event_10_handler(void) { return; };

void TTC_timed_event_11_handler(void) { return; };
void TTC_timed_event_12_handler(void) { return; };
void TTC_timed_event_13_handler(void) { return; };
void TTC_timed_event_14_handler(void) { return; };
void TTC_timed_event_15_handler(void) { return; };
void TTC_timed_event_16_handler(void) { return; };
void TTC_timed_event_17_handler(void) { return; };
void TTC_timed_event_18_handler(void) { return; };
void TTC_timed_event_19_handler(void) { return; };
void TTC_timed_event_20_handler(void) { return; };

void save_buffer_to_SD(csv_t* csv) {
    if (!csv || !csv->data_ptr || csv->cursor == 0) return;
    
    // Open file
    File dataFile = SD.open(csv->file_name_ptr, FILE_WRITE);
    if ( !dataFile ) return;

    // Write headers if not printed yet
    if (!csv->headers_printed) {
        // column_names is a pointer to an array of strings
        char** names = (char**)csv->column_names;
        
        for (int col = 0; col < csv->columns_int; col++) {
            dataFile.print(names[col]);  // Print column name (no "dataFile->" prefix)
            
            if (col < csv->columns_int - 1) {
                dataFile.print(",");
            }
        }
        dataFile.println();
        csv->headers_printed = true;
    }
    
    // Write data rows
    for (int row = 0; row < csv->cursor; row++) {
        for (int col = 0; col < csv->columns_int; col++) {
            double value = csv->data_ptr[(row * csv->columns_int) + col];
            
            // Print the double value
            dataFile.print(value, 3);  // 3 decimal places for temperature
            
            if (col < csv->columns_int - 1) {
                dataFile.print(",");
            }
        }
        dataFile.println();
    }
    
    dataFile.close();
}

void deployment_blinker(void) {
    deploy_current_time = millis();
    deploy_delta_time = deploy_current_time - deploy_last_call_time;
    deploy_last_call_time = deploy_current_time;  // ADD THIS LINE
    deploy_accumulated_time += deploy_delta_time;
    
    switch (deploy_blinker_on) {
        case true:
            if (deploy_accumulated_time < 666) return;
            digitalWrite(LED_PIN, LOW);
            deploy_blinker_on = false;
            deploy_accumulated_time = 0;
            break;
            
        case false:
            if (deploy_accumulated_time < 333) return;
            digitalWrite(LED_PIN, HIGH);
            deploy_blinker_on = true;
            deploy_accumulated_time = 0;
            break;
            
        default:
            return;
    }
}

// Retraction globals
unsigned long retract_next_toggle_time = 0;
bool retract_led_state = false;

void retraction_blinker(void) {
    unsigned long current_time = millis();
    
    // First call - initialize
    if (retract_next_toggle_time == 0) {
        retract_next_toggle_time = current_time + 832;  // Start with OFF period
        digitalWrite(LED_PIN, LOW);
        retract_led_state = false;
        return;
    }
    
    // Check if it's time to toggle
    if (current_time >= retract_next_toggle_time) {
        if (retract_led_state) {
            // Was ON, turn OFF (stay off for 167ms)
            digitalWrite(LED_PIN, LOW);
            retract_next_toggle_time = current_time + 167;
            retract_led_state = false;
        } else {
            // Was OFF, turn ON (stay on for 832ms)
            digitalWrite(LED_PIN, HIGH);
            retract_next_toggle_time = current_time + 832;
            retract_led_state = true;
        }
    }
}

double crunch_flags(bool bool_array[], int size) {

    uint64_t flags = 0ULL;    
    for (int i = 0; i < size && i < 50; i++) {  // 50 bits max for exact double
        if (bool_array[i]) {

            flags |= (1ULL << i);
        }
    }

    return (double)flags;  // Return as double
}

void write_sample_to_csv(csv_t* storage_buffer, sample_t* sample, int first_column_index) {
    if (!storage_buffer || !sample || storage_buffer->cursor >= storage_buffer->max_rows) {
        return;
    }
    
    // Calculate the starting index for the current row
    int row_start_index = storage_buffer->columns_int * storage_buffer->cursor;
    
    //driver state
    storage_buffer->data_ptr[row_start_index + first_column_index] = sample->driver_state;

    //driver errors
    storage_buffer->data_ptr[row_start_index + first_column_index + 1] = sample->driver_error_flags;

    // Write the flags (cast to double)
    storage_buffer->data_ptr[row_start_index + first_column_index + 2] = (double)sample->data_flags;
    
    // Write the sample doubles
    for (int i = 0; i < sample->sample_double_count; i++) {
        storage_buffer->data_ptr[row_start_index + (first_column_index + 3 + i)] = sample->samples[i];
    }
}
