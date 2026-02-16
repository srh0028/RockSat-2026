
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_generic = {

    "Generic Instrument",
    instrument_reading_generic,
    reset_instrument_generic,
    {0.0}};

double instrument_reading_generic(void)
{
    return instrument_generic.pins[ 0 ] + 1;
}

void reset_instrument_generic(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_generic.pins[i] = 0.0;
}