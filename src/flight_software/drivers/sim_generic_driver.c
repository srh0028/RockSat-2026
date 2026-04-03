
#include "flight_software/drivers/sim_generic_driver.h"
#include "flight_software/flight_software_types.h"
#include "simulation/generic_mount.h"
#include <memory.h>

#define GENERIC_SAMPLE_SIZE 1
#define BITS_PER_BYTE 8

instrument_t* driven_instrument = NULL;
data_flag_condition flag_conditions[ BITS_PER_BYTE ] = {

    data_flag_generic_1,
    data_flag_generic_2,
    data_flag_generic_3,
    data_flag_generic_4,
    data_flag_generic_5,
    data_flag_generic_6,
    data_flag_generic_7,
    data_flag_generic_8
};
pin_e motor_pins[ GENERIC_DRIVER_MOTOR_COUNT ] = {

    PIN_19_E,
    PIN_18_E,
    PIN_17_E,
    PIN_16_E,
    PIN_15_E,
    PIN_14_E,
    PIN_13_E,
    PIN_12_E,
};
bool sgd_errors[ SIM_GENERIC_DRIVER_ERROR_COUNT ] = { false };

void initialize_driver( instrument_t* instrument,
                    int target_deployment_inches, 
                    sample_t* sample_buffer,
                    csv_t* storage_buffer ) {

    //I wanted to put this lower in the method but alas
    for ( int i = 0; i < SIM_GENERIC_DRIVER_ERROR_COUNT; i ++ ) sgd_errors[ i ] = false;

    //guard conditions
    if ( instrument->driver_state != D_UNINITIALIZED_E ) {

        sgd_errors[ D_ERROR_REDUNDANT_INITIALIZATION_E ] = true;
    }
    if ( target_deployment_inches > MAX_IMAGINABLE_BOOM_EXTENSION_INCHES ) {

        sgd_errors[ D_ERROR_RIDICULOUS_BOOM_EXTENSION_E ] = true;
    }
    if ( !sample_buffer ) {

        sgd_errors[ D_ERROR_SAMPLE_BUFFER_NPE_E ] = true;
    }
    if ( !storage_buffer ) {

        sgd_errors[ D_ERROR_STORAGE_BUFFER_NPE_E ] = true;
    }

    //initialize and return
    instrument->target_deployment_units = target_deployment_inches;
    instrument->target_deployment_units *= ENCODER_UNITS_PER_INCH; //unit conversion
    sample_buffer->sample_double_count = GENERIC_DOUBLES_PER_SAMPLE;
    instrument->sample_buffer = sample_buffer;
    driven_instrument = instrument;
    driven_instrument->driver_state = D_DEPLOYMENT_E;
}

int generic_deploy_instrumentation(void) {

    //guard conditions
    if ( driven_instrument->driver_state != D_DEPLOYMENT_E ) {

        sgd_errors[ D_ERROR_ILLEGAL_DEPLOYMENT_E ] = true;
    }
    if ( driven_instrument->deployed ) sgd_errors[ D_ERROR_REDUNDANT_DEPLOYMENT_E ] = true;

    //check each motor
    double deployment_buffer = -1.0;
    bool any_motors_still_deploying = false;
    for ( int i = 0; i < GENERIC_DRIVER_MOTOR_COUNT; i ++ ) {

        //break if that motor is deployed successfully
        deployment_buffer = measure_extension( (motor_e) i );
        if ( deployment_buffer >= driven_instrument->target_deployment_units ) continue;

        //otherwise, continue to deploy it
        increment_pin( INSTRUMENT_GENERIC_E, motor_pins[ i ] );
        any_motors_still_deploying = true;
    }

    //check if we are finished and return
    if ( !any_motors_still_deploying ) {

        driven_instrument->deployed = true;
        driven_instrument->driver_state = D_READY_E;
    }
    return 0;
}

int generic_retract_instrumentation(void) {

    //guard conditions
    if ( driven_instrument->driver_state != D_READY_E ) {

        sgd_errors[ D_ERROR_ILLEGAL_RETRACTION_E ] = true;
    }
    if ( driven_instrument->deployed == false ) {

        sgd_errors[ D_ERROR_REDUNDANT_RETRACTION_E ] = true;
    }

    //check each motor
    double deployment_buffer = 1.0;
    bool any_motors_still_retracting = false;
    for ( int i = 0; i < GENERIC_DRIVER_MOTOR_COUNT; i ++ ) {

        //break if that motor is retracted successfully
        deployment_buffer = measure_extension( (motor_e) i );
        if ( deployment_buffer < 1.0 ) continue;

        //otherwise, continue to retract it
        decrement_pin( INSTRUMENT_GENERIC_E, motor_pins[ i ] );
        any_motors_still_retracting = true;
    }

    //check if we are finished and return
    if ( !any_motors_still_retracting ) {

        driven_instrument->deployed = false;
        driven_instrument->driver_state = D_DEPLOYMENT_E;
    }
    return 0;
}

double measure_extension( motor_e which_motor ) {

    double extension = read_pin( INSTRUMENT_GENERIC_E, motor_pins[ which_motor ] );
    return extension;
}

void generic_sample(void) {

    // printf( "%s\n", "generic_sample()" );

    //guard condition
    if ( driven_instrument->driver_state != D_READY_E ) sgd_errors[ D_ERROR_ILLEGAL_SAMPLE_E ] = true;

    //simulate output to a physical instrument
    driven_instrument->driver_state = D_SAMPLING_E;
    update_pin( INSTRUMENT_GENERIC_E, OUTPUT_PIN_1, 68.0 );

    //simulate a sample, process, and populate the struct
    double reading = instrument_reading_generic();
    sample_t* sample_buffer = driven_instrument->sample_buffer;
    sample_buffer->samples[ 0 ] = reading;
    sample_buffer->driver_state = (double) driven_instrument->driver_state;
    sample_buffer->driver_error_flags = crunch_flags( sgd_errors, SIM_GENERIC_DRIVER_ERROR_COUNT );  
    process_sample( sample_buffer );

    // printf( "%s%f\n", "driver state: ", sample_buffer->driver_state );
    // printf( "%s%f\n", "driver error flags: ", sample_buffer->driver_error_flags );

    //squeeze that struct into the storage buffer
    csv_t* storage_buffer = driven_instrument->storage_buffer;
    write_sample_to_csv( storage_buffer, sample_buffer, driven_instrument->first_index_in_csv );

    // printf( "%s\n", "/generic_sample()\n" );

    //update driver state
    driven_instrument->driver_state = D_READY_E;
}

unsigned char process_sample( sample_t* sample ) {

    for ( int i = 0; i < GENERIC_FLAGS_IN_USE; i ++ ) {

        if ( flag_conditions[ i ]( sample ) ) {

            /*
            set the i'th bit from the left using the << left-shift operator
            to 1 if it is 0 using the |= or operator
            IE to set the third flag:
            XXXXX0XX
            OR
            00000100
            =
            XXXXX1XX
            */
            sample->data_flags |= ( 1 << i );
        }
        //otherwise remains 0 by default
    }
}

bool data_flag_generic_1( sample_t* sample ) {

    if ( sample->samples[ 0 ] == 69.0 ) return true;
    return false;
}

bool data_flag_generic_2( sample_t* sample ) {

    return false;
}

bool data_flag_generic_3( sample_t* sample ) {

    return false;
}

bool data_flag_generic_4( sample_t* sample ) {

    return false;
}

bool data_flag_generic_5( sample_t* sample ) {

    return false;
}

bool data_flag_generic_6( sample_t* sample ) {

    return false;
}

bool data_flag_generic_7( sample_t* sample ) {

    return false;
}

bool data_flag_generic_8( sample_t* sample ) {

    return false;
}
