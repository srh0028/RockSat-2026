
#include "langmuir_controller.h"
#include "langmuir_driver.h"
#include "langmuir_types.h"
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <Arduino.h>
// #include <SD.h>
#include <SPI.h>
// #include <Adafruit_MAX31865.h>

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
char current_filename[ MAX_FILENAME_LENGTH ] = { '\0' };

void setup(void) {

    //Link up the function pointers into their respective data structures
    LPC_event_handlers[ 0 ] = LPC_timed_event_1_handler;
    LPC_event_handlers[ 1 ] = LPC_timed_event_2_handler;
    LP_controller.timed_event_handlers = LPC_event_handlers;
    LP_controller.loop = loop;

    //formally initialize the controller
    controller_state_e status = LPC_initialize();

    //presume no errors have occured yet
    for ( int i = 0; i < LPC_ERROR_COUNT; i ++ ) LPC_errors[ i ] = false;

    initialize_pins();

    //initialize storage
    generate_next_filename( current_filename, MAX_FILENAME_LENGTH, LPC_OUTPUT_FILE_NAME );
}

void loop(void) {

    //initialize if necessary (THIS SHOULD NEVER HAPPEN BUT JUST IN CASE?)
    if ( LP_controller.state == C_UNINITIALIZED_E ) LPC_initialize();

    //controller must be initialized by this point. check for timed events
    int status = -1;
    switch ( LPC_environment ) {

        case SIMULATION_E:
            status = LPC_read_in_sim_timed_event();
            break;

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
    pinMode( LED_PIN, OUTPUT );

    //initialize UART/storage
    uart_init( uart0, UART_BAUD_RATE );
    gpio_set_function( UART_TX_PIN, GPIO_FUNC_UART );
    gpio_set_function( UART_RX_PIN, GPIO_FUNC_UART );
    // uart_set_fifo_enabled( uart0, true );
    gpio_init( RECORD_ENABLE_PIN );
    gpio_set_dir( RECORD_ENABLE_PIN, GPIO_OUT );
    gpio_put( RECORD_ENABLE_PIN, HIGH );

    //initialize TE pins
    gpio_init( TE1_PIN );
    gpio_set_dir( TE1_PIN, GPIO_IN );

    gpio_init( TE2_PIN );
    gpio_set_dir( TE2_PIN, GPIO_IN );

    //initialize joint ADC/DAC pins
    gpio_init( CONVERTERS_SCK_PIN );
    gpio_set_dir( CONVERTERS_SCK_PIN, GPIO_OUT );
    // gpio_put( CONVERTERS_SCK_PIN, LOW );

    gpio_init( CONVERTERS_MOSI_PIN );
    gpio_set_dir( CONVERTERS_MOSI_PIN, GPIO_OUT );
    // gpio_put( CONVERTERS_MOSI_PIN, LOW );

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
    // gpio_put( DAC_CS_PIN, HIGH );

    //initialize motor pins
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

    gpio_set_function( MOTOR_PWM_PIN );
    gpio_set_dir( MOTOR_PWM_PIN, GPIO_OUT );
}

void init_servo_pwm(void) {
    gpio_set_function(SERVO_PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PWM_PIN);
    
    // Configure for 50 Hz
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);  // 125 MHz / 125 = 1 MHz counter
    pwm_config_set_wrap(&config, SERVO_PWM_WRAP);  // 20000 = 20ms period
    pwm_init(slice_num, &config, false);
    
    // Start with STOP position
    set_servo_pulse_width(SERVO_PULSE_STOP);
    pwm_set_enabled(slice_num, true);
}

// Helper function to set pulse width in microseconds
void set_servo_pulse_width(uint pulse_width_us) {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PWM_PIN);
    // duty = (pulse_width_us / period_us) * wrap_value
    uint16_t level = (pulse_width_us * SERVO_PWM_WRAP) / 20000;
    pwm_set_chan_level(slice_num, PWM_CHAN_B, level);
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

int LPC_read_in_sim_timed_event(void) {

    // if ( simulation.timed_event_pin > 0 ) return 1;
    return 0;
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

    instrument_t* instrument;
    csv_t* storage_buffer = &LPC_storage_buffer;
    int where = storage_buffer->cursor * storage_buffer->columns_int;

    //Record timestamp
    double clock_dbl = (double) millis();
    storage_buffer->data_ptr[ where ] = clock_dbl;

    //Record timed event
    double timed_event_dbl = 0.0;
    if ( LP_controller.timed_event == NO_TIMED_EVENT_E ) timed_event_dbl = 0;
    else timed_event_dbl = ( (double) LP_controller.timed_event ) + 1;
    storage_buffer->data_ptr[ where + 1 ] = timed_event_dbl;

    //Record controller error flags
    double error_flags_dbl = crunch_flags( LPC_errors, LPC_ERROR_COUNT );
    storage_buffer->data_ptr[ where + 2 ] = error_flags_dbl;

    if ( storage_buffer->cursor >= LPD_SAMPLES_PER_WRITE ) {

        //save the driver's storage buffer to memory if it's time and reset that buffer
        save_buffer_to_SD( storage_buffer );
        storage_buffer->cursor = -1;
        memset( &LPC_data_buffer[ 0 ], 0.0, sizeof( LPC_data_buffer ) );
    }
    instrument->sample();

    //reset controller state
    storage_buffer->cursor ++;
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
    }

    //--> instrument is deployed
    LP_controller.state = C_READY_E;
}

void LPC_retract(void) {

    //guard condition
    if ( LP_controller.state != C_RETRACTING_E ) {

        if ( LP_controller.state != C_READY_E ) return;
    }

    //ensure the instrument is retracting
    /*adjust motor pwm frequency*/
    /*output to the pwm pin*/

    //MANUALLY KILL THE EXPERIMENT'S POWER TO STOP IT FROM BREAKING
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

void save_buffer_to_SD(csv_t* csv) {
    if (!csv || !csv->data_ptr || csv->cursor == 0) return;
    
    // Open file
    File dataFile = SD.open(csv->file_name_ptr, FILE_WRITE);
    if ( !dataFile ) {

        LPC_errors[ C_ERROR_STORAGE_INITIALIZATION_FAILURE_E ] = true;
        return;
    } else LPC_errors[ C_ERROR_STORAGE_INITIALIZATION_FAILURE_E ] = false;

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
        
        LPC_errors[ C_ERROR_SAMPLE_STORAGE_FORMAT_MISMATCH_E ] = true;
    } else LPC_errors[ C_ERROR_SAMPLE_STORAGE_FORMAT_MISMATCH_E ] = false;
    
    // Calculate the starting index for the current row
    int row_start_index = storage_buffer->columns_int * storage_buffer->cursor;
    
    //driver errors
    storage_buffer->data_ptr[row_start_index + first_column_index] = sample->driver_error_flags;

    // Write the sample doubles
    for (int i = 0; i < sample->sample_double_count; i++) {
        storage_buffer->data_ptr[row_start_index + (first_column_index + 1 + i)] = sample->samples[i];
    }
}

void generate_next_filename(char* buffer, int buffer_size, const char* base_name) {
    
    int file_number = 1;
    bool file_exists = true;
    
    while (file_exists) {

        //make the new file name
        snprintf(buffer, buffer_size, "/%s%d.csv", base_name, file_number);
    
        //print that filename to the sd card
        File names_file = SD.open( "names.csv", FILE_WRITE );
        names_file.printf( "file_number: ,%d\nbuffer: %s\n\n", file_number, buffer );
        names_file.close();

        if (SD.exists( buffer )) {
            // File exists
            file_number++;
        } else {
            // File does not exist - we can use this number
            file_exists = false;
        }
        
        // Safety limit
        if (file_number > 999) {
            snprintf(buffer, buffer_size, "%s1.csv", base_name);
            break;
        }
    }
    LPC_storage_buffer.file_name_ptr = buffer;
}
