
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_14 = {

    "Generic instrument Mount 14",
    instrument_reading_14,
    instrument_reading_14,
    { 0.0 }
};

double instrument_reading_14(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_14(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_14.pins[ i ] = 0.0;
}
