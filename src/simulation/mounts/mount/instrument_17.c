
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_17 = {

    "Generic instrument Mount 17",
    instrument_reading_17,
    reset_instrument_17,
    { 0.0 }
};

double instrument_reading_17(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_17(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_17.pins[ i ] = 0.0;
}
