## AI GENERATED ##

cmake_minimum_required(VERSION 3.10)
project(ROCKSAT-2026)

# 1. Enforce C11 Standard (Fixes _Static_assert errors)
set(CMAKE_C_STANDARD 11)        # << ADD THIS
set(CMAKE_C_STANDARD_REQUIRED ON) # << ADD THIS

# 2. Suppress MSVC "Unsafe" Warnings (Fixes C4996 strncpy warnings)
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS) # << ADD THIS
endif()

# Add your include directories
include_directories(include)

# Define sources for the core library (your drivers/controllers)
# NOTE: Update paths if yours are slightly different
set(CORE_SOURCES
    src/simulation/generic_driver.c
    src/simulation/generic_controller.c
    src/core/common/utils.c 
    # Add other necessary .c files here
)

# Create the test executable
add_executable(test_generic_logic test_generic_logic.c ${CORE_SOURCES})

# If on Windows, checking for math library usually isn't needed, 
# but on Linux you might need target_link_libraries(test_generic_logic m)