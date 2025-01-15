cmake_minimum_required(VERSION 3.30)
project(cppThermalCamera VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)

message(STATUS "Found OpenCV ${OpenCV_VERSION}")

if(NOT "${OpenCV_VERSION}" VERSION_EQUAL "4.11.0")
    message(FATAL_ERROR "Incorrect OpenCV version found. Required version 4.11.0")
endif()

add_executable(cppThermalCamera main.cpp)

configure_file(
        "${PROJECT_SOURCE_DIR}/version.h.in"
        "${PROJECT_BINARY_DIR}/version.h"
)

target_include_directories(cppThermalCamera PUBLIC "${PROJECT_BINARY_DIR}")
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(cppThermalCamera ${OpenCV_LIBS})
