
#include "langmuir_driver.h"
#include "langmuir_types.h"
#include "langmuir_controller.h"
// #include "simulation/generic_mount.h"
#include <memory.h>
#include <stdint.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>

#define BITS_PER_BYTE 8

instrument_t* LP_driven_instrument = NULL;
int motor_pins[ LPD_MOTOR_COUNT ] = {

    MOTOR_PIN
};
bool LPD_errors[ LPD_ERROR_COUNT ] = { false };

void LPD_initialize_driver( instrument_t* instrument
                    sample_t* sample_buffer,
                    csv_t* storage_buffer ) {

    //I wanted to put this lower in the method but alas
    for ( int i = 0; i < LPD_ERROR_COUNT; i ++ ) LPD_errors[ i ] = false;

    //guard conditions
    if ( instrument->driver_state != D_UNINITIALIZED_E ) LPD_errors[ D_ERROR_REDUNDANT_INITIALIZATION_E ] = true;
    if ( !sample_buffer ) LPD_errors[ D_ERROR_SAMPLE_BUFFER_NPE_E ] = true;
    if ( !storage_buffer ) LPD_errors[ D_ERROR_STORAGE_BUFFER_NPE_E ] = true;

    //initialize software
    sample_buffer->sample_double_count = LPD_DOUBLES_PER_SAMPLE;
    instrument->sample_buffer = sample_buffer;
    LP_driven_instrument = instrument;
    LP_driven_instrument->driver_state = D_DEPLOYMENT_E;
}

void LPD_deploy_instrumentation(void) {

    //error conditions (never terminal)
    if ( LP_driven_instrument->driver_state != D_DEPLOYMENT_E ) LPD_errors[ D_ERROR_ILLEGAL_DEPLOYMENT_E ] = true;
    else LPD_errors[ D_ERROR_ILLEGAL_DEPLOYMENT_E ] = false;
    if ( LP_driven_instrument->deployed ) LPD_errors[ D_ERROR_REDUNDANT_DEPLOYMENT_E ] = true;
    else LPD_errors[ D_ERROR_REDUNDANT_DEPLOYMENT_E ] = false;

    if ( !LPD_deployment_finished ) {

        /*pwm to the motor pwm pin*/
        return;
    }

    //--> Must have deployed
    LP_driven_instrument->deployed = true;
    LP_driven_instrument->driver_state = D_READY_E;

    /*TURN OFF PWM TO THE MOTOR PWM PIN*/

    /*REINITIALIZE THE MOTOR PWM PIN TO OUTPUT???*/

    return;
}

int LPD_retract_instrumentation(void) {

    //guard conditions
    if ( LP_driven_instrument->driver_state != D_READY_E ) LPD_errors[ D_ERROR_ILLEGAL_RETRACTION_E ] = true;
    else LPD_errors[ D_ERROR_ILLEGAL_RETRACTION_E ] = false;
    if ( LP_driven_instrument->deployed == false ) LPD_errors[ D_ERROR_REDUNDANT_RETRACTION_E ] = true;
    else LPD_errors[ D_ERROR_REDUNDANT_RETRACTION_E ] = false;

    //check each motor
    double deployment_buffer = 1.0;
    bool any_motors_still_retracting = false;
    for ( int i = 0; i < LPD_MOTOR_COUNT; i++ ) {

        //break if that motor is retracted successfully
        deployment_buffer = LPD_measure_extension();
        if ( deployment_buffer < 1.0 ) continue;

        //otherwise, continue to retract it
        extension_motor_units -= 4;
        any_motors_still_retracting = true;
    }

    //check if we are finished and return
    if ( !any_motors_still_retracting ) {

        LP_driven_instrument->deployed = false;
        LP_driven_instrument->driver_state = D_DEPLOYMENT_E;
    }

    return 0;
}

bool LPD_deployment_finished(void) {

    for ( int i = 0; i < MOTOR_COUNT; i ++ ) {

        if ( /*READ IN THE MOTOR PIN*/ ) return false;
    }
    return true;
}

void LPD_sample(void) {

    //error condition (never terminal)
    if ( LP_driven_instrument->driver_state != D_READY_E ) LPD_errors[ D_ERROR_ILLEGAL_SAMPLE_E ] = true;

    //update driver state
    LP_driven_instrument->driver_state = D_SAMPLING_E;

    //populate the sample struct
    sample_t* sample_buffer = LP_driven_instrument->sample_buffer;
    sample_buffer->samples[ 0 ] = /*READ IN ADC_0*/;
    sample_buffer->samples[ 1 ] = /*READ IN ADC_1*/;
    sample_buffer->samples[ 2 ] = /*READ IN ADC_2*/;
    sample_buffer->samples[ 3 ] = /*READ IN ADC_3*/;
    sample_buffer->samples[ 4 ] = /*READ IN ADC_4*/;
    sample_buffer->driver_error_flags = crunch_flags( LPD_errors, LPD_ERROR_COUNT );  

    //squeeze that struct into the storage buffer
    csv_t* storage_buffer = LP_driven_instrument->storage_buffer;
    write_sample_to_csv( storage_buffer, sample_buffer, LP_driven_instrument->first_index_in_csv );

    //update driver state
    LP_driven_instrument->driver_state = D_READY_E;
}
