
#include "simulation/generic_mount.h"

instrument_mount_t instrument_10 = {

    "Generic instrument Mount 10",
    instrument_reading_10,
    reset_instrument_10,
    { 0.0 }
};

double instrument_reading_10(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_10(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_10.pins[ i ] = 0.0;
}
