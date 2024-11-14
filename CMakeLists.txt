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

# Function to create test target
function(create_test_target TEST_NAME TEST_FILE)
    add_custom_target(${TEST_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy ${TEST_FILE} ${CMAKE_BINARY_DIR}/${TEST_NAME}.txt
        COMMAND ${PROJECT_NAME} ${TEST_NAME}.txt
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${PROJECT_NAME}
    )
endfunction()

# Find all test files
file(GLOB TEST_FILES "${TEST_DIR}/test*.txt")

# Create all test targets
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    create_test_target(${TEST_NAME} ${TEST_FILE})
endforeach()

# Add a target to run all tests
add_custom_target(test
    DEPENDS ${PROJECT_NAME}
)

# Make test depend on all individual test targets
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_dependencies(test ${TEST_NAME})
endforeach()

# Create a target to copy all test files
add_custom_target(copy_test_files)

foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME)
    add_custom_command(
        TARGET copy_test_files
        COMMAND ${CMAKE_COMMAND} -E copy ${TEST_FILE} ${CMAKE_BINARY_DIR}/${TEST_NAME}
    )
endforeach()

# Make the executable depend on copying test files
add_dependencies(${PROJECT_NAME} copy_test_files)