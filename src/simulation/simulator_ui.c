
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "simulation/simulator_ui.h"
#include "storage.h"
#include "simulation/real_time_engine.h"

//Simulation state variables
char input_buffer[ MAX_PATH ] = { '\0' };
int status_code_int = NO_SELECTION;
csv_t* user_flight_profile_ptr = NULL;

//menu crap
const char* main_menu_options[] = {

    "Reset to Default",
    "Select Flight Profile",
    "Begin Simulation",
    "Open Output Folder",
    "Open Flight Profile Folder",
    "Help",
    "Debug Menu",
    "Exit Simulator"
};
const char* debug_menu_options[] = {

    "Main Menu",
    "Validate default flight paths",
    "Flip a coin",
    "Sim Snapshot",
    "Stub Option",
    "Stub Option",
    "Stub Option",
    "Stub Option",
    "Stub Option",
};
const char** all_menus[] = {

    main_menu_options,
    debug_menu_options
};
const int menu_sizes[] = {

    sizeof( main_menu_options ) / sizeof( char* ),
    sizeof( debug_menu_options ) / sizeof( char* )
};
const char* menu_names[] = {

    "=====      DASHBOARD      ==",
    "=====      DEBUG MENU     =="
};
const char* help_text = {

    "===== HOW TO USE THE SIMULATOR: ==\n"
    "First, select a flight profile from the folder.\n"
    "Two flight profiles come loaded in the simluator: linear and parabolic.\n"
    "You can also open the flight profile directory to add your own.\n"
    "Resetting the simulator (menu option 1) will replace these profiles, should they be lost.\n"
    "\n"
    "Flight profiles must be .CSV files with 3 columns.\n"
    "The first column contains timing in seconds. Each row is one second of flight.\n"
    "The second column contains altitude in meters.\n"
    "The third column contains timed events. -1 in this column indicates no timed event.\nPositive integers indicate timed events.\n"
    "Up to 20 timed events fit in a flight profile. Each may occur once. They must occur in order.\n"
    "\n"
    "Once you have a profile selected you can begin simulation.\n"
    "The simulator will run your flight software as if it follows the flight profile.\n"
    "When it's finished, the simulator will yield your batch of .CSV files\n"
    "into the 'Simulator Output' folder in its working directory.\n"
    "\n"
};

void initialize_UI(void) {

    //set up file structure
    create_folder( OUTPUT_FOLDER_NAME );
    create_folder( FLIGHT_PROFILES_FOLDER_NAME );

    //replace default flight profiles
    replace_linear_flight_profile();
    replace_parabolic_flight_profile();

    //clear state struct
    user_flight_profile_ptr = NULL;
}

int main_menu(void) {

    while ( 1 ) {

        status_code_int = NO_SELECTION;
        clear_console();
        print_flight_profile_summary();
        print_menu( MAIN_MENU_EV );
        get_input();
        if ( strlen( input_buffer ) == 0 ) continue;
        status_code_int = atoi( input_buffer );
        switch ( status_code_int ) {

            case 1:
                puts( "\nResetting ..." );
                initialize_UI();
                puts( "Reset successful." );
                trip_user();
                break;

            case 2:
                user_selects_flight_profile();
                break;

            case 3:
                puts( "Beginning simulation ..." );
                if ( begin_simulation() ) puts( "Simulation over." );
                trip_user();
                break;

            case 4:
                open_explorer_to_location( OUTPUT_FOLDER_NAME );
                break;

            case 5:
                open_explorer_to_location( FLIGHT_PROFILES_FOLDER_NAME );
                break;

            case 6:
                clear_console();
                printf( "%s", help_text );
                trip_user();
                break;

            case 7:
                status_code_int = debug_menu();
                break;

            case 8:
                clear_console();
                printf( "Exit simulator?\n0) Go Back\n1) Confirm\n\n" );
                get_input();
                int whether = atoi( input_buffer );
                if ( whether == 1 ) exit( 42 );
                puts( "Resuming ..." );
                trip_user();
                break;

            default:
                print_error( "main_menu()" , "Invalid Input", "Please input a number corresponding to a menu option." );
                break;
        }
    }
    return status_code_int;
}

int debug_menu(void) {
    
    while ( 1 ) {

        status_code_int = NO_SELECTION;
        clear_console();
        print_menu( DEBUG_MENU_EV );
        get_input();
        if ( strlen( input_buffer ) == 0 ) continue;
        status_code_int = atoi( input_buffer );
        switch ( status_code_int ) {

            case 1:
                return status_code_int;
                break;

            case 2:
                puts( "\nValidating default flight paths ..." );
                puts( "Loading linear profile ..." );
                csv_t* linear_ptr = read_in_csv_flight_profile( LINEAR_FLIGHT_FILE_NAME );
                puts( "Loading parabolic profile ..." );
                csv_t* parabolic_ptr = read_in_csv_flight_profile( PARABOLIC_FLIGHT_FILE_NAME );
                puts( "Validating linear profile ..." );
                int status_code_1 = validate_flight_profile( linear_ptr );
                printf( "Result: %d\n", status_code_1 );
                puts( "Validating parabolic profile ..." );
                status_code_1 = validate_flight_profile( parabolic_ptr );
                printf( "Result: %d\n", status_code_1 );
                free_csv_t( linear_ptr );
                free_csv_t( parabolic_ptr );
                trip_user();
                break;

            case 3:
                puts( "Coinflip:" );
                int status_code_2 = rand() % 2;
                switch ( status_code_2 ) {

                    case 0:
                        puts( "HEADS" );
                        break;

                    case 1:
                        puts( "TAILS" );
                        break;

                    default:
                        puts( "ERROR" );
                        break;
                }
                trip_user();
                break;

            case 4:
                puts( "\nSim Snapshot:" );
                sim_snapshot();
                break;

            case 5:
                puts( "This option is a stub." );
                trip_user();
                break;

            case 6:
                puts( "This option is a stub." );
                trip_user();
                break;

            case 7:
                puts( "This option is a stub." );
                trip_user();
                break;

            case 8:
                puts( "This option is a stub." );
                trip_user();
                break;
        
            case 9:
                puts( "This option is a stub." );
                trip_user();
                break;

            default:
                print_error( "debug_menu()" , "Invalid Input", "Please input a number corresponding to a menu option." );
                break;
        }
    }
    return status_code_int;
}

char* get_input(void) {

    memset( input_buffer, '\0', INPUT_BUFFER_SIZE );
    fgets( input_buffer, INPUT_BUFFER_SIZE, stdin );
    input_buffer[ strcspn( input_buffer, "\n" ) ] = '\0'; //Remove the newline character
    return input_buffer;
}

void print_menu( menu_e menu ) {

    printf( "\n%s\n", menu_names[ menu ] );
    for ( int i = 0; i < menu_sizes[ menu ]; i ++ ) {

        printf( "%i) %s\n", ( i + 1 ), *( all_menus[ menu ] + i ) );
    }
    printf( "Please make a selection:\r\n" );
}

void print_error( char* calling_method_ptr, char* error_type_ptr, char* error_description_ptr ) {

    puts( "!! EXCEPTION !!!!!" );
    printf( "%s:\n%s:\n%s\n", calling_method_ptr, error_type_ptr, error_description_ptr );
    puts( "!!!!!!!!!!!!!!!!!!" );
    trip_user();
}

void user_selects_flight_profile(void) {

    //set up data, build the search path
    WIN32_FIND_DATA file_data;
    HANDLE file_handle;
    char search_path[ MAX_PATH ];
    char* profile_names[ MAX_FLIGHT_PROFILES_IN_FOLDER ];
    int profile_count_int = 0;
    int user_choice_int;
    int i;
    snprintf( search_path, MAX_PATH, "%s\\*.csv", FLIGHT_PROFILES_FOLDER_NAME );
    
    //grab first profile, guard condition
    file_handle = FindFirstFile( search_path, &file_data );
    if ( file_handle == INVALID_HANDLE_VALUE ) {

        DWORD error_code = GetLastError();
        switch( error_code ) {

            case ERROR_PATH_NOT_FOUND:
                print_error( "user_selects_flight_path()", "Directory Access Exception", "Could not find the flight profile folder. Reset the simulator to get your folder back (menu option 1.)" );
                break;

            case ERROR_FILE_NOT_FOUND:
                print_error( "user_selects_flight_path()", "File Access Exception", "There are no flight profiles in the folder. Reset the simulator to get your default profiles back (menu option 1.)" );
                break;
            
            default:
                print_error( "user_selects_flight_path()", "File Access Exception", "Oops! This error code wasn't accounted for during development." );
                break;
        }
        return;
    }
    
    //grab all profiles and close the file we're trawling
    do {

        //skip directories
        if ( file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) continue;

        // Store the file name
        profile_names[ profile_count_int ] = malloc( strlen( file_data.cFileName ) + 1 );
        if ( profile_names[ profile_count_int ] == NULL ) {

            print_error( "user_selects_flight_profile()", "Memory Allocation Exception", "malloc() failure when allocating memory to print the names of the files in the flight profiles directory." );
            return;
        }

        //replace '.' with null terminator to chop off the file extension
        strcpy( profile_names[ profile_count_int ], file_data.cFileName );
        char *dot_position = strrchr( profile_names[ profile_count_int ], '.' );
        if ( dot_position != NULL ) *dot_position = '\0';  
        profile_count_int++;
    } while ( FindNextFile( file_handle, &file_data ) != 0 );
    FindClose( file_handle );
    
    //guard condition
    if ( profile_count_int == 0 ) {

        printf( "No CSV files found in the '%s' directory.\nReset the simulator to restore default profiles.\n", FLIGHT_PROFILES_FOLDER_NAME );
        return;
    }
    
    //get user input
    int valid_input = 0;
    while ( valid_input == 0 ) {

        //print a menu to the user
        printf( "\n\nAvailable flight profiles:\n" );
        for ( i = 0; i < profile_count_int; i ++ ) printf( "%d) %s\n", i + 1, profile_names[ i ] );

        //grab
        printf( "\nPlease select a flight profile:\n" );
        get_input();
        user_choice_int = atoi( input_buffer );

        //validate
        if ( user_choice_int < 1 || user_choice_int > profile_count_int ) {

            printf( "Invalid selection. Please choose between 1 and %d.\n", profile_count_int );
            continue;
        }
        valid_input = 1;
    }

    //grab and validate the flight profile
    csv_t* result = read_in_csv_flight_profile( profile_names[ user_choice_int - 1 ] );
    int validation_int = validate_flight_profile( result );
    user_validation_feedback( validation_int );
    if ( validation_int != 1 ) {

        free_csv_t( result );
        result = NULL; //ensures state cannot change to an illegal flight profile
    }

    //get me outta here
    for ( i = 0; i < profile_count_int; i++ ) free( profile_names[ i ] );
    user_flight_profile_ptr = result;
}

void print_flight_profile_summary(void) {

    puts( "" );
    if ( user_flight_profile_ptr == NULL ) {

        printf( "No flight profile selected currently.\n" );
        return;
    }
    printf( "Currently Selected Flight Profile:\n%s\n", user_flight_profile_ptr->file_name_ptr );
    double duration_dbl = ( (double)user_flight_profile_ptr->rows_int / 60.0 );
    printf( "Flight Duration (minutes:) %.2f\n", duration_dbl );
}

void clear_console(void) {

    //set up
    HANDLE standard_out_handle = GetStdHandle( STD_OUTPUT_HANDLE );
    COORD coordinates = { 0, 0 };
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO info;
    
    //Guard condition
    if ( !GetConsoleScreenBufferInfo( standard_out_handle, &info ) ) {

        print_error( "clear_console()", "System Call Exception", "Failed to access the windows console buffer using GetConsoleScreenBufferInfo()." );
        return;
    } 

    //clear and reset the buffer
    DWORD cellCount = info.dwSize.X * info.dwSize.Y;
    FillConsoleOutputCharacter( standard_out_handle, ' ', cellCount, coordinates, &count );
    FillConsoleOutputAttribute( standard_out_handle, info.wAttributes, cellCount, coordinates, &count );
    SetConsoleCursorPosition( standard_out_handle, coordinates );
}

void trip_user(void) {

    puts( "Press enter to continue ..." );
    get_input();
}

int validate_flight_profile( csv_t* flight_profile_ptr ) {

    //trivial guard conditions
    if ( !flight_profile_ptr->data_ptr ) return -1;
    if ( !flight_profile_ptr->file_name_ptr ) return -2;
    if ( flight_profile_ptr->rows_int < 1 ) return -3;
    if ( flight_profile_ptr->columns_int != 3 ) return -4;

    //set up to validate each row
    double current_timed_event_dbl = -1.0;
    double current_tick_dbl = -1.0;
    double* current_row_ptr = NULL;
    double third_row_dbl;
    for ( int i = 0; i < flight_profile_ptr->rows_int; i ++ ) {
    
        //grab a row to validate
        current_row_ptr = &( flight_profile_ptr->data_ptr[ ( i * FLIGHT_PROFILE_COLUMNS ) ] );

        //validate first column
        if ( ( current_row_ptr[ 0 ] - current_tick_dbl ) != 1.0 ) return -5; //next tick should be 1 more than current tick
        else current_tick_dbl++;

        //validate second column
        if ( current_row_ptr[ 1 ] < 0 ) return -6; //altitude should never go below 0

        //validate third column
        third_row_dbl = current_row_ptr[ 2 ];
        if ( third_row_dbl == NO_TIMED_EVENT_DOUBLE ) continue; //skip "no timed event" rows
        if ( third_row_dbl > TIMED_EVENT_COUNT ) return -7; //max number of timed events
        if ( third_row_dbl < 0 ) return -7; //timed event should never be negative 
        if ( ( third_row_dbl - current_timed_event_dbl++ ) != 1.0 ) return -7; //timed event should be 1 more than previous timed event
    }

    //if we've survived then it must be valid
    return 1;
}

static void user_validation_feedback( int input_int ) {

    //guard condition
    if ( input_int == 1 ) return;

    //figure out and print the error message
    char error_message[ 133 ] = { '\0' };
    switch ( input_int ) {

        case -1:
            strcpy( error_message, "Failed to read in data from selected flight profile data." );
            break;

        case -2:
            strcpy( error_message, "Failed to read in name of selected flight profile." );
            break;

        case -3:
            strcpy( error_message, "Selected flight profile must have length >=1." );
            break;

        case -4:
            strcpy( error_message, "Selected flight profile must have 3 columns." );
            break;

        case -5:
            strcpy( error_message, "Problem in column 1 of selected flight profile.\nEach row must be timed 1 tick after the previous." );
            break;

        case -6:
            strcpy( error_message, "Problem in column 2 of selected flight profile.\nAltitude must never be negative." );
            break;

        case -7:
            strcpy( error_message, "Problem in column 3 of selected flight profile.\nTimed events must start from 1 and occur in order.\n20 timed events is the maximum." );
            break;

        default:
            strcpy( error_message, "Unspecified error validating selected flight profile." );
            break;
    }
    print_error( "DASHBOARD", "Flight Profile Exception", &( error_message[ 0 ] ) );
}

int begin_simulation() {

    //guard condition
    if ( user_flight_profile_ptr == NULL ) {

        print_error( "begin_simulation()", "Illegal State", "No flight profile is selected." );
        return -1;
    }

    //more guard conditions
    int simulator_code = reset_simulation( user_flight_profile_ptr );
    switch ( simulator_code ) {

        case -1:
            print_error( "begin_simulation()", "Illegal State", "No flight profile is selected." );
            return -2;

        case -2:
            print_error( "begin_simulation()", "Illegal State", "No flight profile is selected." );
            return -3;

        default:
    }
    
    //peepee poopoo
    puts( "Reaching simulation ..." );
    simulate();
}
