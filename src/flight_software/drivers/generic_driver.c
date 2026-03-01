
#include "flight_software/drivers/generic_driver.h"
#include "flight_software/flight_software_types.h"
#include "simulation/generic_mount.h"
#include <memory.h>

#define GENERIC_SAMPLE_SIZE 1
#define BITS_PER_BYTE 8

#define OUTPUT_PIN_1 PIN_0_E

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

int initialize_driver( instrument_t* instrument,
                    int target_deployment_inches, 
                    sample_t* sample_buffer,
                    csv_t* storage_buffer ) {

    //guard conditions
    if ( instrument->driver_state != D_UNINITIALIZED_E ) {

        instrument->driver_state = D_ERROR_E;
        return -1;
    }
    if ( target_deployment_inches > MAX_IMAGINABLE_BOOM_EXTENSION_INCHES ) {

        instrument->driver_state = D_ERROR_E;
        return -2;
    }
    if ( !sample_buffer ) {

        instrument->driver_state = D_ERROR_E;
        return -3;
    }
    if ( !storage_buffer ) {

        instrument->driver_state = D_ERROR_E;
        return -4;
    }

    //initialize and return
    instrument->target_deployment_units = target_deployment_inches;
    instrument->target_deployment_units *= ENCODER_UNITS_PER_INCH; //unit conversion
    sample_buffer->sample_double_count = GENERIC_DOUBLES_PER_SAMPLE;
    instrument->sample_buffer = sample_buffer;
    driven_instrument = instrument;
    driven_instrument->driver_state = D_DEPLOYMENT_E;
    return 1;
}

int generic_deploy_instrumentation(void) {

    //guard conditions
    if ( driven_instrument->driver_state != D_DEPLOYMENT_E ) {

        driven_instrument->driver_state = D_ERROR_E;
        return -1;
    }
    if ( driven_instrument->deployed ) {

        driven_instrument->driver_state = D_ERROR_E;
        return -2;
    }

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
        return 1;
    }
    return 0;
}

int generic_retract_instrumentation(void) {

    //guard conditions
    if ( driven_instrument->driver_state != D_READY_E ) {

        driven_instrument->driver_state = D_ERROR_E;
        return -1;
    }
    if ( driven_instrument->deployed == false ) {

        driven_instrument->driver_state = D_ERROR_E;
        return -2;
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
        return 1;
    }
    return 0;
}

double measure_extension( motor_e which_motor ) {

    double extension = read_pin( INSTRUMENT_GENERIC_E, motor_pins[ which_motor ] );
    return extension;
}

void generic_sample(void) {

    //guard condition
    if ( driven_instrument->driver_state != D_READY_E ) {

        driven_instrument->driver_state = D_ERROR_E;
        return;
    }

    //simulate output to a physical instrument
    driven_instrument->driver_state = D_SAMPLING_E;
    update_pin( INSTRUMENT_GENERIC_E, OUTPUT_PIN_1, 68.0 );

    //simulate a sample, process, and populate the struct
    double reading = instrument_reading_generic();
    time_t sample_time = time(NULL);
    sample_t* sample_buffer = driven_instrument->sample_buffer;
    sample_buffer->timestamp = sample_time;
    sample_buffer->samples[ 0 ] = reading;
    process_sample( sample_buffer );

    //squeeze that struct into the storage buffer
    csv_t* storage_buffer = driven_instrument->storage_buffer;
    int flat_array_index = storage_buffer->columns_int * storage_buffer->cursor;
    storage_buffer->data_ptr[ flat_array_index ] = (double) sample_buffer->timestamp;
    storage_buffer->data_ptr[ flat_array_index + 1 ] = (double) sample_buffer->flags;
    for ( int i = 0; i < GENERIC_DOUBLES_PER_SAMPLE; i++ ) {

        storage_buffer->data_ptr[ flat_array_index + ( i + 2 ) ] = sample_buffer->samples[ i ];
    }
    storage_buffer->cursor++;

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
            sample->flags |= ( 1 << i );
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
