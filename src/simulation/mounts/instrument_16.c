
#include "simulation/generic_mount.h"

instrument_mount_t instrument_16 = {

    "Generic instrument Mount 16",
    instrument_reading_16,
    reset_instrument_16,
    { 0.0 }
};

double instrument_reading_16(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_16(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_16.pins[ i ] = 0.0;
}
