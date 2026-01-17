
#ifndef SIM_UI_H
#define SIM_UI_H

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
char* get_input(void);

/**
 * @brief Prints a UI menu.
 * @param menu Which menu to print
 */
void print_menu( menu_e menu );

/**
 * @brief Prints a detailed error message.
 * @param calling_method_ptr Name of throwing method
 * @param error_type_ptr Title of error
 * @param error_description_ptr Description of error
 */
void print_error( char* calling_method_ptr, char* error_type_ptr, char* error_description_ptr );

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

#endif
