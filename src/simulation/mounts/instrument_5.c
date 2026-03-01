
#include "simulation/generic_mount.h"

instrument_mount_t instrument_5 = {

    "Generic instrument Mount 5",
    instrument_reading_5,
    reset_instrument_5,
    { 0.0 }
};

double instrument_reading_5(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_5(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_5.pins[ i ] = 0.0;
}
