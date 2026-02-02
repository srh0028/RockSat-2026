
#include "simulation/mounts/generic_mount.h"

instrument_mount_t instrument_3 = {

    "Constant instrument",
    instrument_reading_3,
    reset_instrument_3,
    {0.0}};

double instrument_reading_3(void)
{

    return 1.0;
}

void reset_instrument_3(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_3.pins[i] = 0.0;
}