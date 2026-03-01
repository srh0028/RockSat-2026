
#include "simulation/generic_mount.h"
#include "simulation/real_time_engine.h"
#include <stdbool.h>
#include <stdio.h>

bool instrument_in_use[ INSTRUMENT_COUNT ] = {

    true,
    true,
    true,
    true,
    false,
    false,
    false,
    false,
    false,
    false,

    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,

    false
};

instrument_mount_t* instruments[ INSTRUMENT_COUNT ] = {

    &instrument_0,
    &instrument_1,
    &instrument_2,
    &instrument_3,
    &instrument_4,
    &instrument_5,
    &instrument_6,
    &instrument_7,
    &instrument_8,
    &instrument_9,

    &instrument_10,
    &instrument_11,
    &instrument_12,
    &instrument_13,
    &instrument_14,
    &instrument_15,
    &instrument_16,
    &instrument_17,
    &instrument_18,
    &instrument_19,

    &instrument_generic
};

double update_pin( instrument_e instrument, pin_e pin, double new_value_dbl ) {

    double* pin_ptr = &( ( *instruments[ instrument ] ).pins[ pin ] );
    double old_value_dbl = *pin_ptr;
    *pin_ptr = new_value_dbl;
    return old_value_dbl;
}

void increment_pin( instrument_e instrument, pin_e pin ) {

    instruments[ instrument ]->pins[ pin ]++;
}

void decrement_pin( instrument_e instrument, pin_e pin ) {

    instruments[ instrument ]->pins[ pin ]--;
}

void print_instrument( instrument_e instrument ) {

    instrument_mount_t* instrument_ptr = instruments[ instrument ];
    double result_dbl = instrument_ptr->instrument_reading();
    printf( "%s reading: %.2f\n", instrument_ptr->name_ptr, result_dbl );
}

double read_pin( instrument_e instrument, pin_e pin ) {

    return instruments[ instrument ]->pins[ pin ];
}

void print_active_instruments(void) {

    puts( "\nInstrument bank:" );
    for ( int i = 0; i < INSTRUMENT_COUNT; i ++ ) {

        //skip deactivated instruments
        if ( !instrument_in_use[ i ] ) continue;
        print_instrument( ( instrument_e ) i );
    }
    puts( "" );
}

void reset_all_instruments(void) {

    for ( int i = 0; i < INSTRUMENT_COUNT; i ++ ) instruments[ i ]->reset_instrument();
}
