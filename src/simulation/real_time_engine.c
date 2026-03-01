
#include "simulation/real_time_engine.h"
#include "simulation/simulator_ui.h"
#include "simulation/generic_mount.h"
#include "simulation/simulation_config.h"
#include "storage.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

simulation_t simulation = {

    NULL,
    FINISHED_EV,
    TIMED_EVENT_COUNT,
    0,
    -1,
    -1,
    { &generic_controller }
};

int reset_simulation( csv_t* flight_profile_ptr ) {

    //guard conditions
    if ( simulation.status_e == SIMULATING_EV ) {
     
        print_error( "reset_simulation()", "Illegal State", "Cannot reset the simulator while a simluation is in progress." );
        return -1;
    }
    int status_code_int = validate_flight_profile( flight_profile_ptr );
    if ( status_code_int != 1 ) {

        print_error( "reset_simulation()", "Flight Profile Exception", "Selected flight profile is invalid!\nThis should be impossible." );
        return -2;
    }

    //reset the instrument mounts
    reset_all_instruments();

    //set up the simulation struct
    simulation.flight_profile_ptr = flight_profile_ptr;
    simulation.status_e = WAITING_EV;
    simulation.current_timed_event = TIMED_EVENT_COUNT;
    simulation.current_tick_int = 0;
    simulation.current_altitude_dbl = 0.0;

    //set up all the controllers, as happens in real life, and return
    generic_controller_setup();
    return 1;
}

void simulate(void) {

    //guard conditions
    char error_message[ 73 ] = { '\0' };
    if ( simulation.status_e != WAITING_EV ) strcpy( error_message, "Simulation is not in the \"waiting\" state." );
    if ( !simulation.flight_profile_ptr ) strcpy( error_message, "Simulation has no flight profile." );
    if ( simulation.current_timed_event != TIMED_EVENT_COUNT ) strcpy( error_message, "Simulation attempting to start after an in-sim timed event has occured." );
    if ( simulation.current_tick_int != 0 ) strcpy( error_message, "Simulation attempting to start after in-sim time has already passed." );
    if ( simulation.current_altitude_dbl != 0.0 ) strcpy( error_message, "Simulation attempting to start while in-sim altitude is not 0." );
    if ( error_message[ 0 ] != '\0' ) {

        print_error( "simulate()", "Failed to launch simulator", error_message );
        return;
    }

    //main loop
    simulation.status_e = SIMULATING_EV;
    double* current_tick_ptr = NULL;
    double timed_event_dbl = NO_TIMED_EVENT_DOUBLE;
    for ( int i = 0; i < simulation.flight_profile_ptr->rows_int; i ++ ) {

        //update the simulation's variables
        current_tick_ptr = &( simulation.flight_profile_ptr->data_ptr[ ( i * FLIGHT_PROFILE_COLUMNS ) ] );
        simulation.current_tick_int = current_tick_ptr[ 0 ];
        simulation.current_altitude_dbl = current_tick_ptr[ 1 ];
        if ( simulation.timed_event_pin > 0 ) simulation.timed_event_pin--; //ensures there is eventually a falling edge on the TE pin

        //if there is a timed event
        timed_event_dbl = current_tick_ptr[ 2 ];
        if ( timed_event_dbl != NO_TIMED_EVENT_DOUBLE ) {
            
            //update sim state, push timed event to controllers
            int which_event = (int) timed_event_dbl;
            simulation.current_timed_event = (timed_event_e) which_event;
            push_timed_event();

            //feed back to the user
            sim_snapshot();
            print_active_instruments();
            trip_user();
        }

        //loop all controllers
        for ( int i = 0; i < ACTIVE_CONTROLLERS_COUNT; i ++ ) simulation.controllers[ i ]->loop();
    }

    //reset the state
    simulation.status_e = FINISHED_EV;
}

void push_timed_event(void) {

    simulation.timed_event_pin = 10;
}

char* status_strings[ SIMULATOR_STATUS_COUNT ] = {

    "Waiting",
    "Simulating",
    "Finished"
};

char* timed_event_strings[ TIMED_EVENT_COUNT + 1 ] = {

    "Timed Event 1",
    "Timed Event 2",
    "Timed Event 3",
    "Timed Event 4",
    "Timed Event 5",
    "Timed Event 6",
    "Timed Event 7",
    "Timed Event 8",
    "Timed Event 9",
    "Timed Event 10",
    "Timed Event 11",
    "Timed Event 12",
    "Timed Event 13",
    "Timed Event 14",
    "Timed Event 15",
    "Timed Event 16",
    "Timed Event 17",
    "Timed Event 18",
    "Timed Event 19",
    "Timed Event 20",
    "No Timed Event Yet"
};

void sim_snapshot(void) {

    printf( "%s:\n%s, %s\ntick: %d\naltitude: %.2f\n",
        simulation.flight_profile_ptr->file_name_ptr,
        status_strings[ simulation.status_e ],
        timed_event_strings[ simulation.current_timed_event ],
        simulation.current_tick_int,
        simulation.current_altitude_dbl );
}
