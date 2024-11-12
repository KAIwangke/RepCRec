# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name
project(RepCRec)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the source and include directory variables
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")

# Include directories
include_directories(
    ${INCLUDE_DIR}
    ${SOURCE_DIR}
)

# Source files with correct paths
set(SOURCES
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/LockTable.cpp
    ${SOURCE_DIR}/data/DataManager.cpp
    ${SOURCE_DIR}/data/Site.cpp
    ${SOURCE_DIR}/data/Variable.cpp
    ${SOURCE_DIR}/transaction/Transaction.cpp
    ${SOURCE_DIR}/transaction/TransactionManager.cpp
    ${SOURCE_DIR}/transaction/CommandParser.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Find all test files in the test directory
file(GLOB TEST_FILES "${TEST_DIR}/test*.txt")

# Add custom target for running tests
add_custom_target(test)

# Create a test command for each test file
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME)
    add_custom_command(
        TARGET test
        COMMAND ${CMAKE_COMMAND} -E echo "Running ${TEST_NAME}..."
        COMMAND ${CMAKE_COMMAND} -E copy ${TEST_FILE} ${CMAKE_BINARY_DIR}/${TEST_NAME}
        COMMAND ${PROJECT_NAME} ${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

# Make test depend on building the executable
add_dependencies(test ${PROJECT_NAME})