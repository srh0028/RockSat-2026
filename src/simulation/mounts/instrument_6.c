
#include "simulation/generic_mount.h"

instrument_mount_t instrument_6 = {

    "Generic instrument Mount 6",
    instrument_reading_6,
    reset_instrument_6,
    { 0.0 }
};

double instrument_reading_6(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_6(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_6.pins[ i ] = 0.0;
}
