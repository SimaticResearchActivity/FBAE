#
# Lines inspired by https://google.github.io/googletest/quickstart-cmake.html
#

# Note: enable_testing() already done in top CMakeLists.txt

add_executable(tests
        ArgumentsTest.cpp
        SerializationOverheadTest.cpp
        AlgoLayer/SequencerTest.cpp
        AlgoLayer/BBOBBTest.cpp
        AlgoLayer/LCRTest.cpp
        AlgoLayer/TrainsTest.cpp
        CommLayer/TcpTest.cpp
)
target_link_libraries(tests GTest::gtest_main lib_fbae)
if (WIN32)
  target_link_libraries(tests wsock32.lib ws2_32.lib)
else() # We are under UNIX
    target_link_options(tests PRIVATE -pthread)
endif()

add_custom_target(copy-res-tests ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)
add_dependencies(tests copy-res-tests)

# The next two lines enable CMake’s test runner to discover the tests included in the binary,
# using the GoogleTest CMake module.
include(GoogleTest)
gtest_discover_tests(tests)