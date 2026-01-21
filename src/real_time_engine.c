
#include "simulator/real_time_engine.h"
#include "simulator/simulator_ui.h"
#include "simulator/instrument_mount.h"
#include "storage.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

simulation_t simulation = {

    NULL,
    FINISHED_EV,
    TIMED_EVENT_COUNT,
    -1,
    -1
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

    //set up the simulation struct and return
    simulation.flight_profile_ptr = flight_profile_ptr;
    simulation.status_e = WAITING_EV;
    simulation.current_timed_event = TIMED_EVENT_COUNT;
    simulation.current_tick_int = 0;
    simulation.current_altitude_dbl = 0.0;
    puts( "/reset_simulation()" );
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

        //grab the tick data
        current_tick_ptr = &( simulation.flight_profile_ptr->data_ptr[ ( i * FLIGHT_PROFILE_COLUMNS ) ] );

        //update the simulation's variables
        simulation.current_tick_int = current_tick_ptr[ 0 ];
        simulation.current_altitude_dbl = current_tick_ptr[ 1 ];

        //if there is a timed event
        timed_event_dbl = current_tick_ptr[ 2 ];
        if ( timed_event_dbl != NO_TIMED_EVENT_DOUBLE ) {
            
            //update sim state, push timed event to controllers
            int which_event = ( int ) timed_event_dbl;
            simulation.current_timed_event = ( timed_event_e ) which_event;
            push_timed_event( ( timed_event_e ) which_event );

            //feed back to the user
            sim_snapshot();
            print_active_instruments();
            trip_user();
        }
    }
}

void push_timed_event( timed_event_e event_e ) {

    return;
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
