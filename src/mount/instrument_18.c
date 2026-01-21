
#include "simulator/instrument_mount.h"

instrument_mount_t instrument_18 = {

    "Generic instrument Mount 18",
    instrument_reading_18,
    reset_instrument_18,
    { 0.0 }
};

double instrument_reading_18(void) {

    return simulation.current_altitude_dbl;
}

void reset_instrument_18(void) {

    //Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i ++ ) instrument_18.pins[ i ] = 0.0;
}
