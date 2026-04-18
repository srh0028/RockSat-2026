#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_MAX31865.h>

// SD Card pins (SPI0)
#define SD_CS      5
#define SD_MOSI    7
#define SD_MISO    4
#define SD_SCK     6

// MAX31865 pins (SPI1)
#define MAX_CS     13
#define MAX_MOSI   11
#define MAX_MISO   12
#define MAX_SCK    10

// MAX31865 configuration for 3-wire PT100
#define RREF      430.0   // Reference resistor value (430Ω for PT100)
#define RNOMINAL  100.0   // Nominal resistance at 0°C (100Ω for PT100)

// LED
#define LED_PIN    25

// Function declarations
void setupSD(void);
void setupMAX31865(void);
float readTemperature(void);
void writeToSD(String message);
void blinkLED(int times, int delayMs);

#endif