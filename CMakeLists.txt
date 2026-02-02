cmake_minimum_required(VERSION 3.10)
project(ROCKSAT-2026)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Suppress MSVC warnings (if needed for cross-compilation)
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Include directories
include_directories(include)

# Core sources
set(CORE_SOURCES
    src/flight_software/drivers/generic_driver.c
    src/flight_software/controllers/generic_controller.c
    src/core/common/utils.c
    src/core/common/errors.c
    src/core/common/storage.c
    src/core/common/types.c
    src/simulation/simulator_ui.c
    src/simulation/real_time_engine.c
    src/simulation/mounts/generic_mount.c
    src/simulation/mounts/mount/instrument_0.c
    src/simulation/mounts/mount/instrument_1.c
    src/simulation/mounts/mount/instrument_2.c
    src/simulation/mounts/mount/instrument_3.c
    src/simulation/mounts/mount/instrument_4.c
    src/simulation/mounts/mount/instrument_5.c
    src/simulation/mounts/mount/instrument_6.c
    src/simulation/mounts/mount/instrument_7.c
    src/simulation/mounts/mount/instrument_8.c
    src/simulation/mounts/mount/instrument_9.c
    src/simulation/mounts/mount/instrument_10.c
    src/simulation/mounts/mount/instrument_11.c
    src/simulation/mounts/mount/instrument_12.c
    src/simulation/mounts/mount/instrument_13.c
    src/simulation/mounts/mount/instrument_14.c
    src/simulation/mounts/mount/instrument_15.c
    src/simulation/mounts/mount/instrument_16.c
    src/simulation/mounts/mount/instrument_17.c
    src/simulation/mounts/mount/instrument_18.c
    src/simulation/mounts/mount/instrument_19.c
)

# Add test executable
add_executable(test_generic_logic tests/test_generic_logic.c ${CORE_SOURCES})