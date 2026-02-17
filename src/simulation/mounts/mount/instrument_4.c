
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_4 = {

    "Generic instrument Mount 4",
    instrument_reading_4,
    reset_instrument_4,
    { 0.0 }
};

double instrument_reading_4(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_4(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_4.pins[ i ] = 0.0;
}
