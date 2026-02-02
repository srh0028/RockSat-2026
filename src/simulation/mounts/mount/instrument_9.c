
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_9 = {

    "Generic instrument Mount 9",
    instrument_reading_9,
    reset_instrument_9,
    {0.0}};

double instrument_reading_9(void)
{

    return simulation.current_altitude_dbl;
}

void reset_instrument_9(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_9.pins[i] = 0.0;
}