
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_12 = {

    "Generic instrument Mount 12",
    instrument_reading_12,
    reset_instrument_12,
    {0.0}};

double instrument_reading_12(void)
{

    return simulation.current_altitude_dbl;
}

void reset_instrument_12(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_12.pins[i] = 0.0;
}