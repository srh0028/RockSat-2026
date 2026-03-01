
#include "simulation/generic_mount.h"

instrument_mount_t instrument_19 = {

    "Generic instrument Mount 19",
    instrument_reading_19,
    reset_instrument_19,
    { 0.0 }
};

double instrument_reading_19(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_19(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_19.pins[ i ] = 0.0;
}
