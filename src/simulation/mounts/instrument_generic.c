
#include "simulation/generic_mount.h"

instrument_mount_t instrument_generic = {

    "Generic Instrument",
    instrument_reading_generic,
    reset_instrument_generic,
    { 0.0 }
};

double instrument_reading_generic(void) {

    double result = instrument_generic.pins[ 0 ] + 1;
    instrument_generic.pins[ 0 ] /= 2; //mess up the value to make sure the pin is being updated
    return result;
}

void reset_instrument_generic(void) {

    // Reset each pin value to 0
    for ( int i = 0; i < PIN_COUNT; i++ ) instrument_generic.pins[i] = 0.0;
}