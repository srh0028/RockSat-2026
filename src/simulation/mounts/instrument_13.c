
#include "simulation/generic_mount.h"

instrument_mount_t instrument_13 = {

    "Generic instrument Mount 13",
    instrument_reading_13,
    reset_instrument_13,
    { 0.0 }
};

double instrument_reading_13(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_13(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_13.pins[ i ] = 0.0;
}
