
#ifndef INSTRUMENT_MOUNT_H
#define INSTRUMENT_MOUNT_H

#include "simulation/real_time_engine.h"

#define INSTRUMENT_MOUNT_UNFINISHED 0xDEADBEEF

/**
 * @brief Holds a value for each instrument mount.
 */
typedef enum
{

    INSTRUMENT_0_E,
    INSTRUMENT_1_E,
    INSTRUMENT_2_E,
    INSTRUMENT_3_E,
    INSTRUMENT_4_E,
    INSTRUMENT_5_E,
    INSTRUMENT_6_E,
    INSTRUMENT_7_E,
    INSTRUMENT_8_E,
    INSTRUMENT_9_E,

    INSTRUMENT_10_E,
    INSTRUMENT_11_E,
    INSTRUMENT_12_E,
    INSTRUMENT_13_E,
    INSTRUMENT_14_E,
    INSTRUMENT_15_E,
    INSTRUMENT_16_E,
    INSTRUMENT_17_E,
    INSTRUMENT_18_E,
    INSTRUMENT_19_E,

    INSTRUMENT_GENERIC_E,

    INSTRUMENT_COUNT
} instrument_e;

typedef enum
{

    PIN_0_E,
    PIN_1_E,
    PIN_2_E,
    PIN_3_E,
    PIN_4_E,
    PIN_5_E,
    PIN_6_E,
    PIN_7_E,
    PIN_8_E,
    PIN_9_E,

    PIN_10_E,
    PIN_11_E,
    PIN_12_E,
    PIN_13_E,
    PIN_14_E,
    PIN_15_E,
    PIN_16_E,
    PIN_17_E,
    PIN_18_E,
    PIN_19_E,

    PIN_GENERIC_E,

    PIN_COUNT
} pin_e;

/**
 * @brief Struct to represent a physical instrument in the simulation.
 */
typedef struct instrument_mount_t instrument_mount_t;
struct instrument_mount_t
{

    char *name_ptr;
    double (*instrument_reading)(void);
    void (*reset_instrument)(void);
    double pins[PIN_COUNT];
};

extern simulation_t simulation;

extern instrument_mount_t instrument_0;
extern instrument_mount_t instrument_1;
extern instrument_mount_t instrument_2;
extern instrument_mount_t instrument_3;
extern instrument_mount_t instrument_4;
extern instrument_mount_t instrument_5;
extern instrument_mount_t instrument_6;
extern instrument_mount_t instrument_7;
extern instrument_mount_t instrument_8;
extern instrument_mount_t instrument_9;

extern instrument_mount_t instrument_10;
extern instrument_mount_t instrument_11;
extern instrument_mount_t instrument_12;
extern instrument_mount_t instrument_13;
extern instrument_mount_t instrument_14;
extern instrument_mount_t instrument_15;
extern instrument_mount_t instrument_16;
extern instrument_mount_t instrument_17;
extern instrument_mount_t instrument_18;
extern instrument_mount_t instrument_19;

extern instrument_mount_t instrument_generic;

double instrument_reading_0(void);
double instrument_reading_1(void);
double instrument_reading_2(void);
double instrument_reading_3(void);
double instrument_reading_4(void);
double instrument_reading_5(void);
double instrument_reading_6(void);
double instrument_reading_7(void);
double instrument_reading_8(void);
double instrument_reading_9(void);

double instrument_reading_10(void);
double instrument_reading_11(void);
double instrument_reading_12(void);
double instrument_reading_13(void);
double instrument_reading_14(void);
double instrument_reading_15(void);
double instrument_reading_16(void);
double instrument_reading_17(void);
double instrument_reading_18(void);
double instrument_reading_19(void);

double instrument_reading_generic(void);

void reset_instrument_0(void);
void reset_instrument_1(void);
void reset_instrument_2(void);
void reset_instrument_3(void);
void reset_instrument_4(void);
void reset_instrument_5(void);
void reset_instrument_6(void);
void reset_instrument_7(void);
void reset_instrument_8(void);
void reset_instrument_9(void);

void reset_instrument_10(void);
void reset_instrument_11(void);
void reset_instrument_12(void);
void reset_instrument_13(void);
void reset_instrument_14(void);
void reset_instrument_15(void);
void reset_instrument_16(void);
void reset_instrument_17(void);
void reset_instrument_18(void);
void reset_instrument_19(void);

void reset_instrument_generic(void);

/**
 * @brief Updates an instrument mount's pin.
 * @param instrument Instrument to update
 * @param pin Pin to update
 * @param new_value_dbl Value to update it to
 * @returns double value that was replaced
 */
double update_pin(instrument_e instrument, pin_e pin, double new_value_dbl);

void print_instrument(instrument_e instrument);

void print_active_instruments(void);

void reset_all_instruments(void);

#endif