
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_0 = {

    "Altitude Instrument",
    instrument_reading_0,
    reset_instrument_0,
    { 0.0 }
};

double instrument_reading_0(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_0(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_0.pins[ i ] = 0.0;
}
