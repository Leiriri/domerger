cmake_minimum_required(VERSION 4.2.3)
project(dominions-merger)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)

# Define directories
set(SOURCES_DIR src)
set(HEADERS_DIR inc)
set(OBJECTS_DIR obj)

# Collect source and header files
file(GLOB SOURCES "${SOURCES_DIR}/*.cpp")
file(GLOB HEADERS "${HEADERS_DIR}/*.h" "${HEADERS_DIR}/*.hpp")

# Include directories
include_directories(${HEADERS_DIR})

# Compiler flags
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --static")
endif()

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Custom target to run the program
add_custom_target(run
    COMMAND $<TARGET_FILE:${PROJECT_NAME}> -e 2 --log-level=INFO
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
