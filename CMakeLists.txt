# cmake version must be at least 3.26 to provide copy_directory_if_different
cmake_minimum_required(VERSION 3.26)
cmake_policy(VERSION 3.26)

include(FetchContent)

if(WIN32)
  # Variable required to use libraries installed thanks to vcpkg.
  # This variable must be set before "project" cmake instruction.
  set(CMAKE_TOOLCHAIN_FILE C:/Softwares/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

project(FBAE VERSION 1.0.0 LANGUAGES CXX)

if(WIN32)
  # SonarLint support for CMake requires the following flag to be set on.
  #(see https://community.sonarsource.com/t/sonarlint-for-visual-studio-v4-38-support-cmake-projects/50127)
  set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set (BUILD_SHARED_LIBS FALSE)

set(CMAKE_CXX_STANDARD 20)

#
# We include Log4cxx
#
find_package(log4cxx CONFIG REQUIRED)

#
# We generate software using Log4cxx
#

# We want to use Log4cxx format trace capabilities ==> We must set
# "LOG4CXX_FORMAT_NS" compilation flag to "std" value
if (MSVC)
    add_compile_options(/DLOG4CXX_FORMAT_NS=std)
else()
    add_compile_options(-DLOG4CXX_FORMAT_NS=std)
endif()

# Give access to log4cxx include directory
include_directories(PRIVATE $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES>)

#
# Include nlohmann_json
#
find_package(nlohmann_json CONFIG REQUIRED)

#
# Include Yoctolib_cpp
#

# Download and unpack Yoctolib_cpp at configure time
configure_file(CMakeLists_yoctolib_cpp.txt.in yoctolib_cpp-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/yoctolib_cpp-download )
if(result)
    message(FATAL_ERROR "CMake step for yoctolib_cpp failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/yoctolib_cpp-download )
if(result)
    message(FATAL_ERROR "Build step for yoctolib_cpp failed: ${result}")
endif()

# Give access to yoctolib_cpp include directory
include_directories(${CMAKE_CURRENT_BINARY_DIR}/yoctolib_cpp-src/Sources)

# Set
if(WIN32)
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/yoctolib_cpp-src/Sources compiled_yoctolib)
else()
    # Link with the built library    
    add_library(YoctoLib STATIC IMPORTED)
    set(YOCTO_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/yoctolib_cpp-src/Binaries/linux/x86_64/libyocto-static.a)
endif()


#
# Include Cereal (A simple FetchContent does not work ==> We have to download it manually)
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
include_directories(${CMAKE_CURRENT_BINARY_DIR}/cereal-src/include)


#
# Include the subset of Boost which FBAE uses
#
cmake_policy(SET CMP0135 NEW) # This cmake_policy avoids warning by cmake when we fetch contents based on URL
FetchContent_Declare(
    boost_subset_for_asio
    URL https://github.com/SimaticResearchActivity/boost_subset_for_asio/archive/refs/tags/v1.84.0.tar.gz
)
FetchContent_MakeAvailable(boost_subset_for_asio)
if(WIN32)
    # Compilation of some of the boost sources requires the following definition
    # (which is appropriate for Windows 7 target)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()


#
# Include GoogleTest
#
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#
# Generate FBAE targets
#

add_subdirectory(src/core)
add_subdirectory(src/main)

# enable_testing() must be in the source directory root (see cmake documentation at https://cmake.org/cmake/help/latest/ command/enable_testing.html)
# Otherwise, Visual Studio test explorer does not see unit tests (See ticket https://developercommunity.visualstudio.com/t/No-tests-discovered-for-googletest-and-C/1148799#T-ND1150621)
enable_testing()
add_subdirectory(src/test)
