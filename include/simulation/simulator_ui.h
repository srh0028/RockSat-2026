
#ifndef SIM_UI_H
#define SIM_UI_H

#include "storage.h"
#include "simulation/sim_sd.h"

#define INPUT_BUFFER_SIZE MAX_PATH
#define EXIT_CODE 0x5C1B1D11
#define NO_SELECTION 0x80081355
#define MAX_FLIGHT_PROFILES_IN_FOLDER 16

typedef enum {

    MAIN_MENU_EV,
    DEBUG_MENU_EV,

MENU_COUNT
} menu_e;

/**
 * @brief Clears all UI state variables and regenerates default flight profiles.
 */
void initialize_UI(void);

/**
 * @brief Main menu/Dashboard
 * @retval int: Fallthrough exception
 */
int main_menu(void);

/**
 * @brief Debug menu
 * @retval 2: The user chose to return to main menu.
 * @retval Any other int: Fallthrough exception
 */
int debug_menu(void);

/**
 * @brief Helper method to facilitate menus
 */
char *get_input(void);

/**
 * @brief Prints a UI menu.
 * @param menu Which menu to print
 */
void print_menu(menu_e menu);

/**
 * @brief Prints a detailed error message.
 * @param calling_method_ptr Name of throwing method
 * @param error_type_ptr Title of error
 * @param error_description_ptr Description of error
 */
void print_error(char *calling_method_ptr, char *error_type_ptr, char *error_description_ptr);

/**
 * @brief Submenu for the user to select their preferred flight profile.
 */
void user_selects_flight_profile(void);

/**
 * @brief Prints a summary of the current flight profile to the user.
 */
void print_flight_profile_summary(void);

/**
 * @brief Clears the console buffer if possible.
 */
void clear_console(void);

/**
 * @brief Stalls execution until the user presses enter, so they might read a message.
 */
void trip_user(void);

/**
 * @brief Validates a flight profile.
 * @param flight_profile_ptr Profile to validate
 * @retval -1: NULL double* data_ptr
 * @retval -2: NULL char* file_name_ptr
 * @retval -3: <1 rows
 * @retval -4: !3 columns
 * @retval -5: Column 1 invalid
 * @retval -6: Column 2 invalid
 * @retval -7: Column 3 invalid
 * @retval 1: Valid
 */
int validate_flight_profile(csv_t *flight_profile_ptr);

/**
 * @brief Feeds an error message about a flight profile back to the user if necessary.
 */
static void user_validation_feedback(int input_int);

/**
 * @brief Attempts to begin simulation.
 * @retval -1: No flight profile selected
 * @retval -2: Simulation already in progress ( ??? )
 * @retval -3: Invalid flight profile selected
 * @retval 1: Simulation begun
 */
int begin_simulation(void);

#endif