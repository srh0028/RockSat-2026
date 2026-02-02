
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_15 = {

    "Generic instrument Mount 15",
    instrument_reading_15,
    reset_instrument_15,
    {0.0}};

double instrument_reading_15(void)
{

    return simulation.current_altitude_dbl;
}

void reset_instrument_15(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_15.pins[i] = 0.0;
}