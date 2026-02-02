
#include "simulation/mounts/generic_mount.h"

double apogee_dbl = 0.0;

instrument_mount_t instrument_2 = {

    "Apogee Instrument",
    instrument_reading_2,
    reset_instrument_2,
    {0.0}};

double instrument_reading_2(void)
{

    // Do nothing until the second timed event
    if (((int)simulation.current_timed_event) < TIMED_EVENT_3_EV)
        return INSTRUMENT_MOUNT_UNFINISHED;

    // Find the highest altitude reached so far and return it
    double altitude_dbl = simulation.current_altitude_dbl;
    if (altitude_dbl > apogee_dbl)
        apogee_dbl = altitude_dbl;
    return apogee_dbl;
}

void reset_instrument_2(void)
{

    // Reset each pin value to 0
    for (int i = 0; i < PIN_COUNT; i++)
        instrument_2.pins[i] = 0.0;

    // Reset file-scope variable
    apogee_dbl = 0.0;
}