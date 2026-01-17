
#ifndef STORAGE_H
#define STORAGE_H

#define OUTPUT_FOLDER_NAME "Simulator Output"
#define FLIGHT_PROFILES_FOLDER_NAME "Simulator Flight Profiles"
#define FILE_EXTENSION ".csv"

#define FLIGHT_PROFILE_COLUMNS 3

#define TEST_FLIGHTS_DURATION_SECONDS 600 //10 minutes
#define TEST_FLIGHTS_MAX_ALTITUDE_METERS 110000 //10% higher than the edge of space
#define LINEAR_FLIGHT_FILE_NAME "Default Linear"
#define PARABOLIC_FLIGHT_FILE_NAME "Default Parabolic"
#define NO_TIMED_EVENT_DOUBLE -1.0

/**
 * @brief Represents a .csv in memory.
 * @note file_name_ptr and data_ptr MUST be malloc() since they are free() later
 */
typedef struct csv_t csv_t;
struct csv_t {

    char* file_name_ptr;
    int rows_int;
    int columns_int;
    double* data_ptr;
};

/**
 * @brief Replaces a folder if it doesn't exist.
 * @param name Folder to try to create
 */
void create_folder( char* name );

/**
 * @brief Prints the test_csv to the disk.
 */
void test_storage(void);

/**
 * @brief Prints a csv_t to disk as a .csv.
 * @param folder_name_ptr Folder into which to save
 * @param csv_ptr CSV struct to save
 * @retval -1: folder_name_ptr null pointer exception
 * @retval -2: csv_ptr null pointer exception
 * @retval -3: fopen() failure
 * @retval 1: success
 */
int store_csv( char* folder_name_ptr, csv_t* csv_ptr );

/**
 * @brief Reads a flight profile from a .csv to a memory object.
 * @param file_name Name of file to be read in
 * @retval NULL: Failure to open file
 * @retval csv_t*: Success
 */
csv_t* read_in_csv_flight_profile( char* file_name );

/**
 * @brief Returns the number of lines in a csv file.
 * @param folder_name_ptr Where to look for the csv
 * @param file_name_ptr Which file to look for
 * @retval -1: fopen() failure
 * @retval positive int: success
 */
int count_csv_lines( char* folder_name_ptr, char* file_name_ptr );

/**
 * @brief Frees up a csv_t struct's memory.
 * @param csv: csv_t* to free up
 */
void free_csv_t( csv_t* csv );

/**
 * @brief Replaces the linear flight profile .csv.
 */
void replace_linear_flight_profile(void);

/**
 * @brief Replaces the parabolic flight profile .csv.
 */
void replace_parabolic_flight_profile(void);

/**
 * @brief Opens Windows Explorer to the argued directory.
 * @param folder_ptr Name of folder to open
 */
void open_explorer_to_location( char* folder_ptr );

#endif
