cmake_minimum_required(VERSION 3.10)

# Set the project name for the tests
project(NinjaIPCTests)

enable_testing()
include(CTest)

# Specify the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/../src)  # Adjust as needed

# Discover all test files in this directory
file(GLOB TEST_FILES "*.c")

# Create a test executable for each test file
foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)  # Get file name without directory or longest extension
    add_executable(${test_name} ${test_file})
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
