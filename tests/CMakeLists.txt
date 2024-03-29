#
# Lines coming from https://github.com/google/googletest/tree/main/googletest#incorporating-into-an-existing-cmake-project
#

# Download and unpack googletest at configure time
configure_file(CMakeLists_googletest.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

# Now simply link against gtest or gtest_main as needed. Eg
#   enable_testing()  already done in top CMakeLists.txt
add_executable(tests
        testSerializationOverhead.cpp
        SessionLayer/SessionStub.cpp
        SessionLayer/SessionStub.h
        CommLayer/CommStub.cpp
        CommLayer/CommStub.h
        AlgoLayer/testSequencer.cpp
        AlgoLayer/testBBOBB.cpp
)

target_link_libraries(tests gtest_main lib_fbae)
if (WIN32)
  target_link_libraries(tests ws2_32.lib winmm.lib)
else() # We are under UNIX
    target_link_options(tests PRIVATE -pthread)
endif()

add_test(NAME All_fbae_tests COMMAND tests)
