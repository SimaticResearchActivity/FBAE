cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)

include(FetchContent)
project(FBAE VERSION 1.0.0 LANGUAGES CXX)

if(WIN32)
  # SonarLint support for CMake requires the following flag to be set on.
  #(see https://community.sonarsource.com/t/sonarlint-for-visual-studio-v4-38-support-cmake-projects/50127)
  set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set (BUILD_SHARED_LIBS FALSE)

set(CMAKE_CXX_STANDARD 20)

#
# We include the subset of Boost we will be using.
#
include_directories(boost_1_82_0_subset)
if(WIN32)
    # Compilation of some of the boost sources requires the following definition
    # (which is appropriate for Windows 7 target)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

#
# Cereal : A simple FetchContent does not work ==> We generate its library
# in the same manner as google-test
#

# Download and unpack Cereal at configure time
configure_file(CMakeLists_cereal.txt.in cereal-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cereal-download )
if(result)
    message(FATAL_ERROR "CMake step for cereal failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cereal-download )
if(result)
    message(FATAL_ERROR "Build step for cereal failed: ${result}")
endif()

# Give access to cereal include directory
include_directories("${CMAKE_CURRENT_BINARY_DIR}/cereal-src/include")

add_subdirectory(src/lib)
add_subdirectory(src/main)
# enable_testing() must be in the source directory root (see cmake documentation at https://cmake.org/cmake/help/latest/ command/enable_testing.html)
# Otherwise, Visual Studio test explorer does not see unit tests (See ticket https://developercommunity.visualstudio.com/t/No-tests-discovered-for-googletest-and-C/1148799#T-ND1150621)
enable_testing()
add_subdirectory(tests)
