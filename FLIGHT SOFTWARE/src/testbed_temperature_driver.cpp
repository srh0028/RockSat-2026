
#include "testbed_temperature_driver.h"
#include "testbed_types.h"
#include "testbed_temperature_controller.h"
// #include "simulation/generic_mount.h"
#include <memory.h>
#include <stdint.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>

#define GENERIC_SAMPLE_SIZE 1
#define BITS_PER_BYTE 8

int extension_motor_units = 0;
instrument_t* TT_driven_instrument = NULL;
data_flag_condition TTD_flag_conditions[ BITS_PER_BYTE ] = {

    TTD_cold_flag,
    TTD_warm_flag,
    TTD_hot_flag,
    TTD_unused_flag_3,
    TTD_unused_flag_4,
    TTD_unused_flag_5,
    TTD_unused_flag_6,
    TTD_unused_flag_7
};
int motor_pins[ TTD_MOTOR_COUNT ] = {

    MOTOR_PIN
};
bool TTD_errors[ TTD_ERROR_COUNT ] = { false };
// Initialize MAX31865 on SPI1 for 3-wire RTD
Adafruit_MAX31865 max31865 = Adafruit_MAX31865(MAX_CS_PIN, &SPI1);

void TTD_initialize_driver( instrument_t* instrument,
                    int target_deployment_inches, 
                    sample_t* sample_buffer,
                    csv_t* storage_buffer ) {

    //I wanted to put this lower in the method but alas
    for ( int i = 0; i < TTD_ERROR_COUNT; i ++ ) TTD_errors[ i ] = false;

    //guard conditions
    if ( instrument->driver_state != D_UNINITIALIZED_E ) {

        TTD_errors[ D_ERROR_REDUNDANT_INITIALIZATION_E ] = true;
    }
    if ( target_deployment_inches > MAX_IMAGINABLE_BOOM_EXTENSION_INCHES ) {

        TTD_errors[ D_ERROR_RIDICULOUS_BOOM_EXTENSION_E ] = true;
    }
    if ( !sample_buffer ) {

        TTD_errors[ D_ERROR_SAMPLE_BUFFER_NPE_E ] = true;
    }
    if ( !storage_buffer ) {

        TTD_errors[ D_ERROR_STORAGE_BUFFER_NPE_E ] = true;
    }

    //initialize software
    extension_motor_units = 0;
    instrument->target_deployment_units = target_deployment_inches;
    instrument->target_deployment_units *= ENCODER_UNITS_PER_INCH; //unit conversion
    sample_buffer->sample_double_count = TTD_DOUBLES_PER_SAMPLE;
    instrument->sample_buffer = sample_buffer;
    TT_driven_instrument = instrument;
    TT_driven_instrument->driver_state = D_DEPLOYMENT_E;
    
    //initialize MAX31865 pins
    SPI1.setRX(MAX_MISO_PIN);
    SPI1.setTX(MAX_MOSI_PIN);
    SPI1.setSCK(MAX_SCK_PIN);
    SPI1.begin();

    //initialize and configure MAX31865 for 3-wire
    max31865.begin(MAX31865_3WIRE);
    max31865.setWires(MAX31865_3WIRE);
    max31865.enable50Hz(false);
}

int TTD_deploy_instrumentation(void) {

    //error conditions (never terminal)
    if ( TT_driven_instrument->driver_state != D_DEPLOYMENT_E ) TTD_errors[ D_ERROR_ILLEGAL_DEPLOYMENT_E ] = true;
    else TTD_errors[ D_ERROR_ILLEGAL_DEPLOYMENT_E ] = false;
    if ( TT_driven_instrument->deployed ) TTD_errors[ D_ERROR_REDUNDANT_DEPLOYMENT_E ] = true;
    else TTD_errors[ D_ERROR_REDUNDANT_DEPLOYMENT_E ] = false;

    //check each motor
    double deployment_buffer = -1.0;
    bool any_motors_still_deploying = false;
    for ( int i = 0; i < TTD_MOTOR_COUNT; i ++ ) {

        //break if that motor is deployed successfully
        deployment_buffer = TTD_measure_extension();
        if ( deployment_buffer >= TT_driven_instrument->target_deployment_units ) continue;

        //otherwise, continue to deploy it
        extension_motor_units += 4;
        any_motors_still_deploying = true;
    }

    //check if we are finished and return
    if ( !any_motors_still_deploying ) {

        TT_driven_instrument->deployed = true;
        TT_driven_instrument->driver_state = D_READY_E;
    }

    return 0;
}

int TTD_retract_instrumentation(void) {

    //guard conditions
    if ( TT_driven_instrument->driver_state != D_READY_E ) TTD_errors[ D_ERROR_ILLEGAL_RETRACTION_E ] = true;
    else TTD_errors[ D_ERROR_ILLEGAL_RETRACTION_E ] = false;
    if ( TT_driven_instrument->deployed == false ) TTD_errors[ D_ERROR_REDUNDANT_RETRACTION_E ] = true;
    else TTD_errors[ D_ERROR_REDUNDANT_RETRACTION_E ] = false;

    //check each motor
    double deployment_buffer = 1.0;
    bool any_motors_still_retracting = false;
    for ( int i = 0; i < TTD_MOTOR_COUNT; i++ ) {

        //break if that motor is retracted successfully
        deployment_buffer = TTD_measure_extension();
        if ( deployment_buffer < 1.0 ) continue;

        //otherwise, continue to retract it
        extension_motor_units -= 4;
        any_motors_still_retracting = true;
    }

    //check if we are finished and return
    if ( !any_motors_still_retracting ) {

        TT_driven_instrument->deployed = false;
        TT_driven_instrument->driver_state = D_DEPLOYMENT_E;
    }

    return 0;
}

double TTD_measure_extension(void) {

    return extension_motor_units;
}

void TTD_sample(void) {

    //error condition (never terminal)
    if ( TT_driven_instrument->driver_state != D_READY_E ) TTD_errors[ D_ERROR_ILLEGAL_SAMPLE_E ] = true;

    //update driver state
    TT_driven_instrument->driver_state = D_SAMPLING_E;

    // read raw value from MAX31865 over SPI
    uint16_t raw_rtd = max31865.readRTD();

    //catch errors
    if ( raw_rtd == 0xFFFF ) {

        uint8_t fault = max31865.readFault();
        if ( fault & MAX31865_FAULT_HIGHTHRESH ) TTD_errors[ D_FAULT_HIGH_THRESH_E ] = true;
        if ( fault & MAX31865_FAULT_LOWTHRESH ) TTD_errors[ D_FAULT_LOW_THRESH_E ] = true;
        if ( fault & MAX31865_FAULT_REFINLOW ) TTD_errors[ D_FAULT_REF_IN_LOW_E ] = true;
        if ( fault & MAX31865_FAULT_REFINHIGH ) TTD_errors[ D_FAULT_REF_IN_HIGH_E ] = true;
        if ( fault & MAX31865_FAULT_RTDINLOW ) TTD_errors[ D_FAULT_RTD_IN_LOW_E ] = true;
        if ( fault & MAX31865_FAULT_OVUV ) TTD_errors[ D_FAULT_OVER_UNDER_VOLTAGE_E ] = true;
        max31865.clearFault();
    }

    // Calculate resistance
    double ratio = raw_rtd / 32768.0;
    double resistance = RTD_REF_OHMS * ratio;

    //Linear approximation
    double temperature = ( resistance - PT100_RESISTANCE_0C ) / PT100_ALPHA;

    //populate the sample struct
    sample_t* sample_buffer = TT_driven_instrument->sample_buffer;
    sample_buffer->samples[ 0 ] = raw_rtd;
    sample_buffer->samples[ 1 ] = temperature;
    sample_buffer->driver_state = (double) TT_driven_instrument->driver_state;
    sample_buffer->driver_error_flags = crunch_flags( TTD_errors, TTD_ERROR_COUNT );  
    TTD_process_sample( sample_buffer );

    //squeeze that struct into the storage buffer
    csv_t* storage_buffer = TT_driven_instrument->storage_buffer;
    write_sample_to_csv( storage_buffer, sample_buffer, TT_driven_instrument->first_index_in_csv );

    //update driver state
    TT_driven_instrument->driver_state = D_READY_E;
}

void TTD_process_sample( sample_t* sample ) {

    for ( int i = 0; i < TTD_FLAGS_IN_USE; i ++ ) {

        if ( TTD_flag_conditions[ i ]( sample ) ) {

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
        //This clears it to 0. Don't ask me how it works
        else sample->data_flags &= ~(1 << i);
    }
}

bool TTD_cold_flag( sample_t* sample ) {

    if ( sample->samples[ 1 ] < 15.0 ) return true;
    return false;
}

bool TTD_warm_flag( sample_t* sample ) {

    double temp = sample->samples[ 0 ];
    if ( ( temp > 15.0 ) && ( temp < 25.0 ) ) return true;
    return false;
}

bool TTD_hot_flag( sample_t* sample ) {

    if ( sample->samples[ 1 ] > 25.0 ) return true;
    return false;
}

bool TTD_unused_flag_3( sample_t* sample ) {

    return false;
}

bool TTD_unused_flag_4( sample_t* sample ) {

    return false;
}

bool TTD_unused_flag_5( sample_t* sample ) {

    return false;
}

bool TTD_unused_flag_6( sample_t* sample ) {

    return false;
}

bool TTD_unused_flag_7( sample_t* sample ) {

    return false;
}
