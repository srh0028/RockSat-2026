
#include "langmuir_controller.h"
#include "langmuir_driver.h"
#include "langmuir_types.h"
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include "hardware/pwm.h"

environment_e LPC_environment = FLIGHT_E;
timed_event_handler LPC_event_handlers[ TIMED_EVENT_COUNT ] = {

    LPC_timed_event_1_handler,
    LPC_timed_event_2_handler
};
sample_t LPC_sample_buffer = {

    0.0,
    { 0.0 },
    0,
    0,
    0.0,
    0.0
};
double LPC_data_buffer[ LPC_DATA_BUFFER_SIZE ] = { 0.0 };
char* LPC_csv_column_names[ LPC_STORAGE_COLUMNS ] = {

    "millis",
    "timed event",
    "controller errors",
    "driver errors",
    "ADC housekeeping",
    "p1V",
    "p1I",
    "p2V",
    "p2I"
};
csv_t LPC_storage_buffer = {

    NULL,
    0,
    LPC_STORAGE_COLUMNS,
    LPC_csv_column_names,
    false,
    &LPC_data_buffer[ 0 ],
    LPD_SAMPLES_PER_WRITE,
    0
};
instrument_t LP_instrument = {

    D_UNINITIALIZED_E,
    LPD_deploy_instrumentation,
    LPD_retract_instrumentation,
    LPD_sample,
    -1,
    false,
    { false },
    &LPC_sample_buffer,
    &LPC_storage_buffer,
    LPC_PERIPHERAL_COLUMNS //starts printing in the CSV after the controller's peripheral columns
};
controller_t LP_controller = {

    NO_TIMED_EVENT_E,
    C_UNINITIALIZED_E,
    { NULL },
    NULL,
    false,
    NULL
};
bool LPC_errors[ LPC_ERROR_COUNT ] = { false };

void setup(void) {

    //hardware initialization
    initialize_pins();
    Serial.begin( UART_BAUD_RATE );
    for ( int i = 0; i < LPC_STORAGE_COLUMNS; i ++ ) Serial.printf( "%s,", LPC_csv_column_names[ i ] );
    Serial.println();
    digitalWrite( LED_PIN, LVL_HIGH );

    //Link up the function pointers into their respective data structures
    LPC_event_handlers[ 0 ] = LPC_timed_event_1_handler;
    LPC_event_handlers[ 1 ] = LPC_timed_event_2_handler;
    LP_controller.timed_event_handlers = LPC_event_handlers;
    LP_controller.loop = loop;

    //formally initialize the controller
    controller_state_e status = LPC_initialize();

    //presume no errors have occured yet
    for ( int i = 0; i < LPC_ERROR_COUNT; i ++ ) LPC_errors[ i ] = false;
}

void loop(void) {

    //initialize if necessary (THIS SHOULD NEVER HAPPEN BUT JUST IN CASE?)
    if ( LP_controller.state == C_UNINITIALIZED_E ) LPC_initialize();

    //controller must be initialized by this point. check for timed events
    int status = -1;
    switch ( LPC_environment ) {

        // case SIMULATION_E:
        //     status = LPC_read_in_sim_timed_event();
        //     break;

        case FLIGHT_E:
            status = LPC_read_in_flight_timed_event();
            break;

        default:
            LPC_errors[ C_ERROR_ENVIRONMENT_FALLTHROUGH_E ] = true;
            return;
    }

    //adopt new timed events
    if ( status != LP_controller.timed_event ) LP_controller.timed_event = (timed_event_e) status;

    //finally, perform the actions slated for the timed event we are in
    if ( LP_controller.timed_event != NO_TIMED_EVENT_E ) LP_controller.timed_event_handlers[ LP_controller.timed_event ]();
    else {

        //perform events that occur before any timed event
        return;
    }
}

void initialize_pins(void) {

    //initialize UI
    gpio_init( LED_PIN );
    gpio_set_dir( LED_PIN, GPIO_OUT );

    //initialize UART/storage
    uart_init( uart0, UART_BAUD_RATE );
    gpio_set_function( UART_TX_PIN, GPIO_FUNC_UART );
    gpio_set_function( UART_RX_PIN, GPIO_FUNC_UART );
    // uart_set_fifo_enabled( uart0, true );

    gpio_init( RECORD_ENABLE_PIN );
    gpio_set_dir( RECORD_ENABLE_PIN, GPIO_OUT );
    gpio_put( RECORD_ENABLE_PIN, LVL_LOW );

    //initialize TE pins
    gpio_init( TE1_PIN );
    gpio_set_dir( TE1_PIN, GPIO_IN );

    gpio_init( TE2_PIN );
    gpio_set_dir( TE2_PIN, GPIO_IN );

    //initialize joint ADC/DAC pins
    gpio_init( CONVERTERS_SCK_PIN );
    gpio_set_dir( CONVERTERS_SCK_PIN, GPIO_OUT );
    // gpio_put( CONVERTERS_SCK_PIN, LVL_LOW );

    gpio_init( CONVERTERS_MOSI_PIN );
    gpio_set_dir( CONVERTERS_MOSI_PIN, GPIO_OUT );
    // gpio_put( CONVERTERS_MOSI_PIN, LVL_LOW );

    //initialize ADC pins
    gpio_init( ADC_MISO_0_PIN );
    gpio_set_dir( ADC_MISO_0_PIN, GPIO_IN );
    // gpio_put(  );

    gpio_init( ADC_MISO_1_PIN );
    gpio_set_dir( ADC_MISO_1_PIN, GPIO_IN );
    // gpio_put(  );

    gpio_init( ADC_MISO_2_PIN );
    gpio_set_dir( ADC_MISO_2_PIN, GPIO_IN );
    // gpio_put(  );

    gpio_init( ADC_MISO_3_PIN );
    gpio_set_dir( ADC_MISO_3_PIN, GPIO_IN );
    // gpio_put(  );

    gpio_init( ADC_MISO_4_PIN );
    gpio_set_dir( ADC_MISO_4_PIN, GPIO_IN );
    // gpio_put(  );

    gpio_init( ADC_CNV_PIN );
    gpio_set_dir( ADC_CNV_PIN, GPIO_OUT );
    // gpio_put(  );

    gpio_init( ADC_BUSY_PIN );
    gpio_set_dir( ADC_BUSY_PIN, GPIO_IN );
    // gpio_put(  );

    //initialize DAC pin
    gpio_init( DAC_CS_PIN );
    gpio_set_dir( DAC_CS_PIN, GPIO_OUT );
    // gpio_put( DAC_CS_PIN, LVL_HIGH );

    //initialize motor limit switch pins
    gpio_init( MOTOR_PIN_0 );
    gpio_set_dir( MOTOR_PIN_0, GPIO_IN );

    gpio_init( MOTOR_PIN_1 );
    gpio_set_dir( MOTOR_PIN_1, GPIO_IN );

    gpio_init( MOTOR_PIN_2 );
    gpio_set_dir( MOTOR_PIN_2, GPIO_IN );

    gpio_init( MOTOR_PIN_3 );
    gpio_set_dir( MOTOR_PIN_3, GPIO_IN );

    gpio_init( MOTOR_PIN_4 );
    gpio_set_dir( MOTOR_PIN_4, GPIO_IN );

    gpio_init( MOTOR_PIN_5 );
    gpio_set_dir( MOTOR_PIN_5, GPIO_IN );

    //PWM ~~~NOT FINISHED~~~ ==!!!!!!!==
    // gpio_set_function( MOTOR_PWM_PIN );
    // gpio_set_dir( MOTOR_PWM_PIN, GPIO_OUT );

    initialize_PWM( TEST_MOTOR_PIN );

    //initialize 

}

void initialize_PWM( int which_pin ) {

    // 1. Configure the GPIO pin for PWM function
    gpio_set_function(which_pin, GPIO_FUNC_PWM);
    
    // 2. Get the PWM slice and channel for this GPIO pin
    uint pwm_slice_num = pwm_gpio_to_slice_num(which_pin);
    uint pwm_channel = pwm_gpio_to_channel(which_pin);
    
    // 3. Configure the PWM period for 50Hz (20ms period)
    // Set clock divider so counter increments every 1 microsecond
    pwm_set_clkdiv(pwm_slice_num, PWM_CLOCK_DIVIDER);
    pwm_set_wrap(pwm_slice_num, PWM_PERIOD_US);
    
    // 4. Set initial pulse width to stop position (1500µs)
    pwm_servo_set_pulse_us(TEST_MOTOR_PIN, MOTOR_PWM_STATIONARY);
    
    // 5. Enable the PWM output
    pwm_set_enabled(pwm_slice_num, true);
}

void pwm_servo_set_pulse_us( uint which_pin, uint pulse_width_us ) {

    // Get the PWM slice and channel for this GPIO pin
    uint slice_num = pwm_gpio_to_slice_num(which_pin);
    uint channel = pwm_gpio_to_channel(which_pin);
    
    // Calculate duty cycle level:
    // level = (pulse_width / period) * wrap_value
    uint32_t level = (pulse_width_us * PWM_PERIOD_US) / 20000;
    pwm_set_chan_level(slice_num, channel, level);
}

controller_state_e LPC_initialize(void) {

    //Initialize a driver on top of the statically allocated instrument
    LPD_initialize_driver( &LP_instrument, 
                        &LPC_sample_buffer, 
                        &LPC_storage_buffer );

    //plug all my instruments into the array
    LP_controller.instruments[ 0 ] = &LP_instrument;

    //update and return
    LP_controller.state = C_UNDEPLOYED_E;
    return C_UNDEPLOYED_E;
}

int LPC_read_in_flight_timed_event(void) {

    if ( BOOTSEL ) return 1;
    return 0;
}

void LPC_sample_cycle(void) {

    //guard condition
    if ( LP_controller.state != C_READY_E ) LPC_errors[ C_ERROR_UNPREPARED_TO_SAMPLE_E ] = true;
    else LPC_errors[ C_ERROR_UNPREPARED_TO_SAMPLE_E ] = false;

    //Begin sample cycle
    LP_controller.state = C_SAMPLING_E;

    //Record timestamp
    uint clock_int = (double) millis();

    //Record timed event
    short timed_event_sht = 0;
    if ( LP_controller.timed_event == NO_TIMED_EVENT_E ) timed_event_sht = 0;
    else timed_event_sht = ( (int) LP_controller.timed_event ) + 1;

    //Record error flags
    int controller_error_flags_int = crunch_flags( LPC_errors, LPC_ERROR_COUNT );
    int driver_error_flags_int = crunch_flags( LP_instrument.errors, LPD_ERROR_COUNT );

    //sample, update, and return
    LP_instrument.sample();
    LP_controller.state = C_READY_E;
}

void LPC_deploy(void) {

    //guard condition
    if ( LP_controller.state == C_UNINITIALIZED_E ) {

        LPC_errors[ C_ERROR_DEPLOYMENT_WHILE_UNINITIALIZED_E ] = true;
        LPC_initialize();
    }

    //if the instrument is not deployed yet
    if ( LP_instrument.deployed == false ) {

        //make sure it gets the deployment signal and return
        LP_controller.state = C_DEPLOYING_E;
        LP_instrument.deploy();
        return;
    }

    //--> instrument is deployed
    LP_controller.state = C_READY_E;
}

void LPC_retract(void) {

    // //guard condition
    // if ( LP_controller.state != C_RETRACTING_E ) {

    //     if ( LP_controller.state != C_READY_E ) return;
    // }

    //ensure the instrument is retracting
    /*adjust motor pwm frequency*/
    /*output to the pwm pin*/

    //MANUALLY KILL THE EXPERIMENT'S POWER TO STOP IT FROM BREAKING

    //for testing purposes:

    LP_instrument.retract();

}

//DEPLOY, SAMPLE
void LPC_timed_event_1_handler(void) {

    if ( LP_controller.state == C_READY_E ) {

        LPC_sample_cycle();
        return;
    }
    LPC_deploy();
    LPC_sample_cycle();
}

//RETRACT
void LPC_timed_event_2_handler(void) {
    
    LPC_retract();
}

int crunch_flags(bool bool_array[], int size) {

    uint64_t flags = 0ULL;    
    for (int i = 0; i < size && i < 32; i++) {  // 32 bits max for exact int
        if (bool_array[i]) {

            flags |= (1ULL << i);
        }
    }

    return (int) flags;  // Return as int
}
