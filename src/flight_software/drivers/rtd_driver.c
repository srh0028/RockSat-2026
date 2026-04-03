// since this is a windows repo i can't use real pico sdk calls. This file contains stub versions that return a fake temp
// so project compiles and runs

// replace the stubs with actual SPI calls, then flash it to the pico and see if we get real temp readings 

#include "flight_software/drivers/rtd_driver.h"
#include <stdint.h>

/**
 * @brief Stub init — on real hardware this sets up SPI and configures MAX31865.
 * doesn't do shit in sim
 */
void rtd_driver_init(void) {

    // no-op in simulation
}

/**
 * @brief Stub raw read. On real hardware reads 15-bit value from MAX31865 over SPI.
 * In simulation returns a fixed raw value corresponding to ~25 degrees C.
 */
int rtd_read_raw(uint16_t* out_raw) {

    if (!out_raw) return -1;

    // raw value for ~25C:
    // resistance = 100 + (25 * 100 * 0.00385) = 109.625 ohms
    // raw = (109.625 / 430) * 32768 = 8354
    *out_raw = 8354;
    return 0;
}

/**
 * @brief Stub fault check aalways returns no fault in simulation.
 */
int rtd_check_fault(uint8_t* out_fault) {

    if (!out_fault) return -1;
    *out_fault = 0;
    return 0;
}

/**
 * @brief Converts raw 15-bit ADC value to resistance in ohms.
 */
float rtd_raw_to_resistance(uint16_t raw) {

    return ((float)raw / 32768.0f) * RTD_REF_OHMS;
}

/**
 * @brief Converts resistance in ohms to temperature in Celsius.
 * Uses linearized Callendar-Van Dusen, accurate to ~0.5C.
 */
float rtd_resistance_to_celsius(float resistance) {

    return (resistance - RTD_NOMINAL_OHMS) / (RTD_NOMINAL_OHMS * RTD_ALPHA);
}