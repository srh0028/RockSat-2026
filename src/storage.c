
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "storage.h"
#include "simulation/simulator_ui.h"
#include "simulation/real_time_engine.h"
#include "simulation/sim_sd.h"

#if defined(_WIN32)
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

void create_folder( char* name_ptr ) {

    //guard condition
    if ( !name_ptr ) {

        print_error( "create_folder()" , "Null Pointer Exception", "char* name_ptr is NULL. Bug in the calling code?" );
        return;
    }

    //Set up folder name and filepath
    char current_directory[ MAX_PATH ] = { '\0' };
    char folder_name[ MAX_PATH ] = { '\0' };
    memcpy( folder_name, name_ptr, strlen( name_ptr ) );
    GetCurrentDirectory( MAX_PATH, current_directory );
    
    //attempt to create the directory, throw an error if it fails
    int status_code = _mkdir( folder_name );
    if ( ( status_code != 0 ) || ( errno == EEXIST ) ) {

        //TODO CAUSE A CENTRALIZED ERROR HERE
        return;
    }
}

int store_csv( char* folder_name_ptr, csv_t* csv_ptr ) {

    //guard conditions
    if ( folder_name_ptr == NULL ) {

        print_error( "store_csv()" , "Null Pointer Exception", "char* folder_name_ptr is NULL. Bug in calling code?" );
        return -1;
    }
    if ( csv_ptr == NULL ) {

        print_error( "store_csv()" , "Null Pointer Exception", "csv_t* csv_ptr is NULL. Bug in calling code?" );
        return -2;
    }

    //confirm the destination folder exists. Build the filepath
    create_folder( folder_name_ptr );
    char file_path[ MAX_PATH ] = { '\0' };
    snprintf( file_path, sizeof( file_path ), "%s/%s%s", folder_name_ptr, csv_ptr->file_name_ptr, FILE_EXTENSION );

    //open and validate the file
    FILE* file = fopen( file_path, "w" );
    if ( !file ) {

        print_error( "store_csv()", "File Access Exception", "Failed to open a file to save a csv to memory." );
        return -3;
    }

    //iterate through every value to be printed to the file
    for ( int row = 0; row < csv_ptr->rows_int; row ++ ) {

        for ( int column = 0; column < csv_ptr->columns_int; column ++ ) {

            //print the value to the file, add delimiter if not the last column
            fprintf( file, "%.3f", csv_ptr->data_ptr[ ( row * csv_ptr->columns_int ) + column ] );
            if ( column < ( csv_ptr->columns_int - 1 ) ) fprintf( file, "," );
        }

        //add newline at the end of each line
        fprintf( file, "\n" );
    }
    
    //clean up after yourself
    fclose( file );
    return 1;
}

csv_t* read_in_csv_flight_profile( char* file_name_ptr ) {

    //Grab the file and validate it
    char file_path[ MAX_PATH ] = { '\0' };
    snprintf( file_path, sizeof( file_path ), "%s/%s%s", FLIGHT_PROFILES_FOLDER_NAME, file_name_ptr, FILE_EXTENSION );
    FILE* file = fopen( file_path, "r" );
    if ( !file ) {

        print_error( "read_in_csv_flight_profile()", "File Access Exception", "Failed to open a flight profile csv file." );
        return NULL;
    }

    //measure the file and try a guard condition
    int how_many_lines_int = count_csv_lines( FLIGHT_PROFILES_FOLDER_NAME, file_name_ptr );
    if ( how_many_lines_int == 0 ) {

        print_error( "read_in_csv_flight_profile()", "Illegal Input Exception", "Cannot parse a flight profile of length 0." );
        fclose( file );
        return NULL;
    }

    //allocate the result struct
    #define NUMBER_OF_FLIGHT_PROFILE_COLUMNS 3
    csv_t* result = malloc( sizeof( csv_t ) );
    if ( !result ) {

        print_error( "read_in_csv_flight_profile()", "Memory Allocation Exception", "Failed to allocate memory to store a csv_t." );
        return NULL;
    }
    int how_much_string_memory_int = ( strlen( file_name_ptr ) * sizeof( char ) ); 
    char* csv_t_name_ptr = malloc( how_much_string_memory_int );
    if ( !csv_t_name_ptr ) {

        print_error( "read_in_csv_flight_profile()", "Memory Allocation Exception", "Failed to allocate memory to store filename in a csv_t." );
        return NULL;
    }
    int how_much_double_memory_int = how_many_lines_int * NUMBER_OF_FLIGHT_PROFILE_COLUMNS * sizeof( double );
    double* csv_t_data_ptr = malloc( how_much_double_memory_int );
    if ( !csv_t_data_ptr ) {

        print_error( "read_in_csv_flight_profile()", "Memory Allocation Exception", "Failed to allocate memory to store data in a csv_t." );
        return NULL;
    }
    result->file_name_ptr = csv_t_name_ptr;
    result->data_ptr = csv_t_data_ptr;
    
    //populate the result struct
    strcpy( result->file_name_ptr, file_name_ptr );
    result->columns_int = NUMBER_OF_FLIGHT_PROFILE_COLUMNS;
    result->rows_int = how_many_lines_int;
    for ( int line = 0; line < how_many_lines_int; line ++ ) {

        //Grab all three values at once
        fscanf( file, "%lf,%lf,%lf",
            &result->data_ptr[ ( line * 3 ) + 0 ],
            &result->data_ptr[ ( line * 3 ) + 1 ],
            &result->data_ptr[ ( line * 3 ) + 2 ] );
    }

    //clean up after yourself you fuckin barn animal
    fclose( file );
    return result;
}

int count_csv_lines( char* folder_name_ptr, char* file_name_ptr ) {

    //grab and validate the file
    char file_path[ MAX_PATH ] = { '\0' };
    snprintf( file_path, sizeof( file_path ), "%s/%s%s", folder_name_ptr, file_name_ptr, FILE_EXTENSION );
    FILE* file = fopen( file_path, "r" );
    if ( !file ) {

        print_error( "count_csv_lines()", "File Access Exception", "Failed to open the csv file to measure." );
        return -1;
    }

    //count lines in the file
    #define LINE_COUNTING_BUFFER_SIZE 100
    char buffer[ LINE_COUNTING_BUFFER_SIZE ] = { '\0' };
    int how_many_lines = 0;
    while ( fgets( buffer, LINE_COUNTING_BUFFER_SIZE, file ) != NULL ) how_many_lines++;

    //Return the sum
    fclose( file );
    return how_many_lines;
}

void free_csv_t( csv_t* csv_ptr ) {

    if ( !csv_ptr ) return;
    free( csv_ptr->data_ptr );
    free( csv_ptr->file_name_ptr );
    free( csv_ptr );
}

void replace_linear_flight_profile(void) {

    //build array of doubles corresponding to the flight profile
    double* profile = malloc( sizeof( double ) * TEST_FLIGHTS_DURATION_SECONDS * FLIGHT_PROFILE_COLUMNS );
    if ( !profile ) {

        print_error( "replace_linear_flight_profile()", "Memory Allocation Exception", "Failed to allocate memory to store data for a csv_t." );
        return;
    }
    int i;
    double timed_events = 0.0;
    double buffer;
    int halfway = ( TEST_FLIGHTS_DURATION_SECONDS - 1 ) / 2;
    for ( i = 0; i < halfway; i ++ ) {

        //first half goes up
        buffer = TEST_FLIGHTS_MAX_ALTITUDE_METERS * ( (double) i / (double) halfway );
        profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 0 ] = i;
        profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 1 ] = buffer;
        if ( ( i % ( TEST_FLIGHTS_DURATION_SECONDS / TIMED_EVENT_COUNT ) == 0 ) && ( i != 0 ) ) profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 2 ] = timed_events++;
        else profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 2 ] = NO_TIMED_EVENT_DOUBLE;
    }
    for ( i; i < TEST_FLIGHTS_DURATION_SECONDS; i ++ ) {

        //second half goes down
        buffer = TEST_FLIGHTS_MAX_ALTITUDE_METERS * ( 1.0 - (double)( i - halfway ) / ( TEST_FLIGHTS_DURATION_SECONDS - 1 - halfway ) );
        profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 0 ] = i;
        profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 1 ] = buffer;
        if ( i % ( TEST_FLIGHTS_DURATION_SECONDS / TIMED_EVENT_COUNT ) == 0 ) profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 2 ] = timed_events++;
        else profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 2 ] = NO_TIMED_EVENT_DOUBLE;
    }

    //fill out the csv struct to be saved
    csv_t* result = malloc( sizeof( csv_t ) );
    if ( !result ) {

        print_error( "replace_linear_flight_profile()", "Memory Allocation Exception", "Failed to allocate memory to store a csv_t." );
        return;
    }
    int how_much_string_memory_int = ( ( strlen( LINEAR_FLIGHT_FILE_NAME ) + 1 ) * sizeof( char ) );
    char* csv_t_name_ptr = malloc( how_much_string_memory_int );
    if ( !csv_t_name_ptr ) {

        print_error( "replace_linear_flight_profile()", "Memory Allocation Exception", "Failed to allocate memory to store filename in a csv_t." );
        return;
    }
    strcpy( csv_t_name_ptr, LINEAR_FLIGHT_FILE_NAME );
    result->file_name_ptr = csv_t_name_ptr;
    result->columns_int = FLIGHT_PROFILE_COLUMNS;
    result->rows_int = TEST_FLIGHTS_DURATION_SECONDS;
    result->data_ptr = profile;

    //save that csv to the flight profiles folder
    store_csv( FLIGHT_PROFILES_FOLDER_NAME, result );
    free_csv_t( result );
}

void replace_parabolic_flight_profile(void) {

    //build array of doubles corresponding to the flight profile
    double* profile = malloc( sizeof( double ) * TEST_FLIGHTS_DURATION_SECONDS * FLIGHT_PROFILE_COLUMNS );    double timed_events = 0.0;
    if ( !profile ) {

        print_error( "replace_parabolic_flight_profile()", "Memory Allocation Error", "Failed to allocate memory for data in a csv_t." );
        return;
    }
    double buffer;
    for ( int i = 0; i < TEST_FLIGHTS_DURATION_SECONDS; i ++ ) {

        // Parabolic formula: y = 4 * max_height * t * (1 - t)
        buffer = 4.0 * TEST_FLIGHTS_MAX_ALTITUDE_METERS * 
                 i * ( TEST_FLIGHTS_DURATION_SECONDS - 1 - i ) / 
                 ( ( TEST_FLIGHTS_DURATION_SECONDS - 1 ) * ( TEST_FLIGHTS_DURATION_SECONDS - 1 ) );
        profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 0 ] = i;
        profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 1 ] = buffer;
        if ( ( i % ( TEST_FLIGHTS_DURATION_SECONDS / TIMED_EVENT_COUNT ) == 0 ) && ( i != 0 ) ) profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 2 ] = timed_events++;
        else profile[ ( i * FLIGHT_PROFILE_COLUMNS ) + 2 ] = NO_TIMED_EVENT_DOUBLE;
    }

    //fill out the csv struct to be saved
    csv_t* result = malloc( sizeof( csv_t ) );
    if ( !result ) {

        print_error( "replace_parabolic_flight_profile()", "Memory Allocation Error", "Failed to allocate memory for a csv_t." );
        return;
    }
    int how_much_string_memory_int = ( ( strlen( PARABOLIC_FLIGHT_FILE_NAME ) + 1 ) * sizeof( char ) );
    char* csv_t_name_ptr = malloc( how_much_string_memory_int );
    if ( !csv_t_name_ptr ) {

        print_error( "replace_parabolic_flight_profile()", "Memory Allocation Exception", "Failed to allocate memory to store filename in a csv_t." );
        return;
    }
    strcpy( csv_t_name_ptr, PARABOLIC_FLIGHT_FILE_NAME );
    result->file_name_ptr = csv_t_name_ptr;
    result->columns_int = FLIGHT_PROFILE_COLUMNS;
    result->rows_int = TEST_FLIGHTS_DURATION_SECONDS;
    result->data_ptr = profile;

    //save that csv to the flight profiles folder
    store_csv( FLIGHT_PROFILES_FOLDER_NAME, result );
    free_csv_t( result );
}

void open_explorer_to_location( char* folder_ptr ) {

    char current_directory[ MAX_PATH ] = { '\0' };
    char total_path[ MAX_PATH ] = { '\0' };
    GetCurrentDirectory( MAX_PATH, current_directory );
    snprintf( total_path, MAX_PATH, "%s\\%s", current_directory, folder_ptr );
    ShellExecuteA( NULL, "open", "explorer.exe", total_path, NULL, SW_SHOWNORMAL );
}

void save_buffer_to_sim_sd(csv_t* csv) {
    if (!csv || !csv->data_ptr || csv->cursor == 0) return;
    
    // Open file
    File* dataFile = SD.open(csv->file_name_ptr, FILE_WRITE);
    
    if (dataFile) {
        // Write headers if not printed yet
        if (!csv->headers_printed) {
            // column_names is a pointer to an array of strings
            char** names = (char**)csv->column_names;
            
            for (int col = 0; col < csv->columns_int; col++) {
                dataFile->print(dataFile, names[col]);  // Print column name
                
                if (col < csv->columns_int - 1) {
                    dataFile->print(dataFile, ",");
                }
            }
            dataFile->println(dataFile);
            csv->headers_printed = true;
        }
        
        // Write data rows
        for (int row = 0; row < csv->cursor; row++) {
            for (int col = 0; col < csv->columns_int; col++) {
                double value = csv->data_ptr[(row * csv->columns_int) + col];
                
                // Print the double value
                dataFile->print_float(dataFile, value);
                
                if (col < csv->columns_int - 1) {
                    dataFile->print(dataFile, ",");
                }
            }
            dataFile->println(dataFile);
        }
        
        dataFile->close(dataFile);
    }
}

double crunch_flags(bool bool_array[], int size) {

    // printf( "%s\n", "crunch_flags()" );

    uint64_t flags = 0ULL;

    // printf( "%s%f\n", "flags (initial:) ", (double) flags );
    
    for (int i = 0; i < size && i < 53; i++) {  // 53 bits max for exact double
        if (bool_array[i]) {

            // printf( "%s%d\n", "flipping flag: ", i );

            flags |= (1ULL << i);
        }
    }
    
    // printf( "%s%f\n", "flags (final:) ", (double) flags );

    return (double)flags;  // Return as double
}

void write_sample_to_csv(csv_t* storage_buffer, sample_t* sample, int first_column_index) {
    if (!storage_buffer || !sample || storage_buffer->cursor >= storage_buffer->max_rows) {
        return;
    }
    
    // Calculate the starting index for the current row
    int row_start_index = storage_buffer->columns_int * storage_buffer->cursor;
    
    // Write the flags (cast to double)
    storage_buffer->data_ptr[row_start_index + first_column_index] = (double)sample->data_flags;
    
    // Write the sample doubles
    for (int i = 0; i < sample->sample_double_count; i++) {
        storage_buffer->data_ptr[row_start_index + (first_column_index + 1 + i)] = sample->samples[i];
    }
}
