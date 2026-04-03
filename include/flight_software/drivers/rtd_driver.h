#ifndef RTD_DRIVER_H
#define RTD_DRIVER_H

#include <stdint.h>

// SPI pins
#define RTD_PIN_SCK     18 // clock wire
#define RTD_PIN_MOSI    19 // Pico - > MAX Command and config
#define RTD_PIN_MISO    16 // Max -> Pico, sends tempd data back
#define RTD_PIN_CS      17 // chip select // pico pulls this LOW to say "start talking"

// MAX31865 registers (Comments ai gen but cross referenced with documentation)
#define MAX31865_REG_CONFIG     0x00  // configuration slot — tells chip how to operate
                                      // (3-wire mode, bias on, filter frequency etc.)

#define MAX31865_REG_RTD_MSB    0x01  // most significant byte of the temperature reading
                                      // (the upper 8 bits of the 15-bit result)

#define MAX31865_REG_RTD_LSB    0x02  // least significant byte of the temperature reading
                                      // (the lower 8 bits, last bit is a fault flag)

#define MAX31865_REG_FAULT      0x07  // fault status slot — tells you what went wrong
                                      // if a fault bit was set (open wire, short, etc.)

#define MAX31865_WRITE_BIT      0x80  // this is 10000000 in binary
                                      // MAX31865 requires you OR this onto any register
                                      // address when writing to it, to distinguish
                                      // writes from reads

#define MAX31865_CONFIG_3WIRE   0b11000011  // the actual value written to REG_CONFIG
                                            // each bit is a setting:
                                            // bit 7: V_BIAS on
                                            // bit 6: auto-convert mode on
                                            // bit 4: 3-wire mode on
                                            // bit 0: 50Hz filter (use 0 for 60Hz)


// PT100 constants
#define RTD_NOMINAL_OHMS   100.0f   // PT100 reads exactly 100 ohms at 0°C
                                    // this is where the "100" in PT100 comes from

#define RTD_REF_OHMS       430.0f   // the reference resistor soldered onto the
                                    // MAX31865 breakout board — Adafruit uses 430 ohms
                                    // for PT100. The chip measures your sensor as a
                                    // ratio against this known resistor

#define RTD_ALPHA          0.00385f // how much resistance changes per degree C
                                    // per ohm of nominal resistance
                                    // PT100 gains 0.385 ohms for every 1°C increase
                                    // this is defined by the IEC 60751 standard

void    rtd_driver_init(void);
int     rtd_read_raw(uint16_t* out_raw);
int     rtd_check_fault(uint8_t* out_fault);
float   rtd_raw_to_resistance(uint16_t raw);
float   rtd_resistance_to_celsius(float resistance);

#endif
