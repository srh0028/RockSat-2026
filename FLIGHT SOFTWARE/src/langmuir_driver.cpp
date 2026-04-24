
#include "langmuir_driver.h"
#include "langmuir_types.h"
#include "langmuir_controller.h"
#include <memory.h>
#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>

#define BITS_PER_BYTE 8

instrument_t* LP_driven_instrument = NULL;
uint32_t raw_ADC_data[ ADC_CHANNELS_IN_USE ] = { 0 };
size_t raw_data_size;
int16_t signed_data_buffer = 0;

void LPD_initialize_driver( instrument_t* instrument,
                    sample_t* sample_buffer,
                    csv_t* storage_buffer ) {

    LP_driven_instrument = instrument;

    //I wanted to put this lower in the method but alas
    for ( int i = 0; i < LPD_ERROR_COUNT; i ++ ) LP_driven_instrument->errors[ i ] = false;

    //guard conditions
    if ( instrument->driver_state != D_UNINITIALIZED_E ) LP_driven_instrument->errors[ D_ERROR_REDUNDANT_INITIALIZATION_E ] = true;
    if ( !sample_buffer ) LP_driven_instrument->errors[ D_ERROR_SAMPLE_BUFFER_NPE_E ] = true;
    if ( !storage_buffer ) LP_driven_instrument->errors[ D_ERROR_STORAGE_BUFFER_NPE_E ] = true;

    //initialize software
    instrument->sample_buffer = sample_buffer;
    LP_driven_instrument = instrument;
    LP_driven_instrument->driver_state = D_UNDEPLOYED_E;
    raw_data_size = sizeof( raw_ADC_data );
}

void LPD_deploy_instrumentation(void) {

    //error conditions (never terminal)
    if ( LP_driven_instrument->driver_state != D_UNDEPLOYED_E ) LP_driven_instrument->errors[ D_ERROR_ILLEGAL_DEPLOYMENT_E ] = true;
    else LP_driven_instrument->errors[ D_ERROR_ILLEGAL_DEPLOYMENT_E ] = false;
    if ( LP_driven_instrument->deployed ) LP_driven_instrument->errors[ D_ERROR_REDUNDANT_DEPLOYMENT_E ] = true;
    else LP_driven_instrument->errors[ D_ERROR_REDUNDANT_DEPLOYMENT_E ] = false;

    //If I am not deployed
    if ( !LP_driven_instrument->deployed ) {

        //If I have not yet begun to deploy
        if ( LP_driven_instrument->driver_state != D_DEPLOYING_E ) {

            //Begin deployment and update state and return
            LP_driven_instrument->driver_state = D_DEPLOYING_E;
            pwm_servo_set_pulse_us( TEST_MOTOR_PIN, MOTOR_PWM_DEPLOYMENT );
            return;
        }
        //Otherwise I must already be deploying so I should check if I'm finished
        if ( !gpio_get( TEST_LIMIT_SWITCH_PIN ) ) { //high by default, goes low when tripped

            LP_driven_instrument->driver_state = D_READY_E;
            LP_driven_instrument->deployed = true;
            pwm_servo_set_pulse_us( TEST_MOTOR_PIN, MOTOR_PWM_STATIONARY );
        }
    }
}

void LPD_retract_instrumentation(void) {

    //guard conditions
    if ( LP_driven_instrument->driver_state != D_READY_E ) LP_driven_instrument->errors[ D_ERROR_ILLEGAL_RETRACTION_E ] = true;
    else LP_driven_instrument->errors[ D_ERROR_ILLEGAL_RETRACTION_E ] = false;
    if ( LP_driven_instrument->deployed == false ) LP_driven_instrument->errors[ D_ERROR_REDUNDANT_RETRACTION_E ] = true;
    else LP_driven_instrument->errors[ D_ERROR_REDUNDANT_RETRACTION_E ] = false;

    // //check each motor
    // double deployment_buffer = 1.0;
    // bool any_motors_still_retracting = false;
    // for ( int i = 0; i < LPD_MOTOR_COUNT; i++ ) {

    //     //break if that motor is retracted successfully
    //     deployment_buffer = LPD_measure_extension();
    //     if ( deployment_buffer < 1.0 ) continue;

    //     //otherwise, continue to retract it
    //     extension_motor_units -= 4;
    //     any_motors_still_retracting = true;
    // }

    // //check if we are finished and return
    // if ( !any_motors_still_retracting ) {

    //     LP_driven_instrument->deployed = false;
    //     LP_driven_instrument->driver_state = D_UNDEPLOYED_E;
    // }

    pwm_servo_set_pulse_us( TEST_MOTOR_PIN, MOTOR_PWM_RETRACTION );
}

void LPD_sample(void) {

    //set up for sampling
    if ( LP_driven_instrument->driver_state != D_READY_E ) LP_driven_instrument->errors[ D_ERROR_ILLEGAL_SAMPLE_E ] = true;
    else LP_driven_instrument->errors[ D_ERROR_ILLEGAL_SAMPLE_E ] = false;
    memset( raw_ADC_data, 0, raw_data_size );
    
    //send a pulse to tell the ADC to convert
    gpio_put( ADC_CNV_PIN, 1 );
    delayMicroseconds( 1 );
    gpio_put( ADC_CNV_PIN, 0 );

    //wait for and confirm ADC has finished conversion
    delayMicroseconds( 5 );
    if ( !gpio_get( ADC_BUSY_PIN ) ) { //ADC_BUSY goes LOW when conversion is complete

        //chip select
        gpio_put( ADC_CS_PIN, 0 );
        delayMicroseconds( 1 );

        //for each bit of data
        for ( int i = 23; i >= 0; i-- ) {

            //clock out each data channel
            gpio_put( CONVERTERS_SCK_PIN, 1 );
            delayMicroseconds( 1 );
            raw_ADC_data[ 0 ] |= gpio_get( ADC_MISO_0_PIN ) << i;
            raw_ADC_data[ 1 ] |= gpio_get( ADC_MISO_1_PIN ) << i;
            raw_ADC_data[ 2 ] |= gpio_get( ADC_MISO_2_PIN ) << i;
            raw_ADC_data[ 3 ] |= gpio_get( ADC_MISO_3_PIN ) << i;
            raw_ADC_data[ 4 ] |= gpio_get( ADC_MISO_4_PIN ) << i;
            gpio_put( CONVERTERS_SCK_PIN, 0 );
            delayMicroseconds( 1 );
        }
    }
    delayMicroseconds( 1 );
    gpio_put( ADC_CS_PIN, 1 ); //turn off the lights when you leave the room
    
    //for each sample
    signed_data_buffer = 0;
    for ( int i = 0; i < ADC_CHANNELS_IN_USE; i ++ ) {

        //discard the leading byte
        signed_data_buffer = (int16_t) ( raw_ADC_data[ i ] >> 8 ) & 0xFFFF;

        //send the remaining two bytes to the SD logger
        Serial.printf( "%d,", signed_data_buffer );
    }
    
    //close the line of the file
    Serial.println();
  }
