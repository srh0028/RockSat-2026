
#include "simulator/instrument_mount.h"

instrument_mount_t instrument_11 = {

    "Generic instrument Mount 11",
    instrument_reading_11,
    reset_instrument_11,
    { 0.0 }
};

double instrument_reading_11(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_11(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_11.pins[ i ] = 0.0;
}
