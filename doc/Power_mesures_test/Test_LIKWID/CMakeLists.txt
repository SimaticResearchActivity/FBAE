cmake_minimum_required(VERSION 3.28)
project(Test_LIKWID)

set(CMAKE_CXX_STANDARD 20)

#
# Include likwid
#
# Find the package
find_path(LIKWID_INCLUDE_DIR NAMES likwid.h PATHS /usr/local/include /usr/include)
find_library(LIKWID_LIBRARY NAMES likwid PATHS /usr/local/lib /usr/lib)

if(NOT LIKWID_INCLUDE_DIR OR NOT LIKWID_LIBRARY)
    message(FATAL_ERROR "Could not find LIKWID library or include directory")
endif()

include_directories(${LIKWID_INCLUDE_DIR})

add_executable(Test_LIKWID main.cpp)

target_link_libraries(Test_LIKWID ${LIKWID_LIBRARY})