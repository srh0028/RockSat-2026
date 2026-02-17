
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_7 = {

    "Generic instrument Mount 7",
    instrument_reading_7,
    reset_instrument_7,
    { 0.0 }
};

double instrument_reading_7(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_7(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_7.pins[ i ] = 0.0;
}
