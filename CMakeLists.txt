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