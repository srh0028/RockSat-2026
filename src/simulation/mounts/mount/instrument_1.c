
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_1 = {

    "Negative Altitude instrument",
    instrument_reading_1,
    reset_instrument_1,
    {0.0}};

double instrument_reading_1(void)
{

    return -1.0 * simulation.current_altitude_dbl;
}

void reset_instrument_1(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_1.pins[i] = 0.0;
}