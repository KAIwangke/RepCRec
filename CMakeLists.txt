# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name
project(RepCRec)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set directories
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(TEST_STD_DIR "${PROJECT_SOURCE_DIR}/test_std")

# Include directories
include_directories(
    ${INCLUDE_DIR}
    ${SOURCE_DIR}
)

# Source files
set(SOURCES
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/data/DataManager.cpp
    ${SOURCE_DIR}/data/Site.cpp
    ${SOURCE_DIR}/data/Variable.cpp
    ${SOURCE_DIR}/transaction/Transaction.cpp
    ${SOURCE_DIR}/transaction/TransactionManager.cpp
    ${SOURCE_DIR}/transaction/CommandParser.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Function to create a test target for each test file
function(create_test_target TEST_NAME TEST_FILE)
    # This target:
    # 1. Copies the input test file into the build directory.
    # 2. Prints "Running testN..."
    # 3. Runs the test, piping output to both screen and testN_output.txt using tee.
    # No diffing at this stage; the diff will be done separately if desired.
    add_custom_target(${TEST_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy ${TEST_FILE} ${CMAKE_BINARY_DIR}/${TEST_NAME}.txt
        COMMAND ${CMAKE_COMMAND} -E echo "Running ${TEST_NAME}..."
        COMMAND /bin/sh -c "./${PROJECT_NAME} ${TEST_NAME}.txt | tee ${TEST_NAME}_output.txt"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${PROJECT_NAME}
        VERBATIM
        USES_TERMINAL
    )
endfunction()

# Find all test files
file(GLOB TEST_FILES "${TEST_DIR}/test*.txt")

# Create test targets for each discovered test file
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    create_test_target(${TEST_NAME} ${TEST_FILE})
endforeach()

# Add a target 'test' that runs all tests
add_custom_target(test
    DEPENDS ${PROJECT_NAME}
)
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_dependencies(test ${TEST_NAME})
endforeach()

# Create a diff_all target that just performs diffs without re-running tests
add_custom_target(diff_all
    COMMENT "Running diff for all tests..."
)

# For each test, add a diff command
# This will:
# - Check if both the expected file and the output file exist.
# - If both exist, run diff.
# - If missing, print a message.
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_custom_command(
        TARGET diff_all
        POST_BUILD
        COMMAND /bin/sh -c "if [ -f ${TEST_STD_DIR}/${TEST_NAME}.txt ] && [ -f ${TEST_NAME}_output.txt ]; then diff ${TEST_STD_DIR}/${TEST_NAME}.txt ${TEST_NAME}_output.txt; else echo 'No std or output for ${TEST_NAME}'; fi"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Diffing ${TEST_NAME}.txt"
        VERBATIM
    )
endforeach()

# Optional: A target to copy all test files if needed.
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
