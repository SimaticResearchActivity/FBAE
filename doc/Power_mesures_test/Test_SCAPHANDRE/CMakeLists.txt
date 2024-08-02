cmake_minimum_required(VERSION 3.28)
project(Test_SCAPHANDRE)

set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(Test_SCAPHANDRE main.cpp)
set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

target_link_libraries(Test_SCAPHANDRE PRIVATE nlohmann_json::nlohmann_json)
