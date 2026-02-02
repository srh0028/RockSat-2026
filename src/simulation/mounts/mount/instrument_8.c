
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_8 = {

    "Generic instrument Mount 8",
    instrument_reading_8,
    reset_instrument_8,
    {0.0}};

double instrument_reading_8(void)
{

    return simulation.current_altitude_dbl;
}

void reset_instrument_8(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_8.pins[i] = 0.0;
}