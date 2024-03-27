#
# Lines inspired by https://google.github.io/googletest/quickstart-cmake.html
#

# Note: enable_testing() already done in top CMakeLists.txt

add_executable(tests
        testSerializationOverhead.cpp
        SessionLayer/SessionStub.cpp
        SessionLayer/SessionStub.h
        CommLayer/CommStub.cpp
        CommLayer/CommStub.h
        AlgoLayer/testSequencer.cpp
        AlgoLayer/testBBOBB.cpp
)
target_link_libraries(tests GTest::gtest_main lib_fbae)
if (WIN32)
  target_link_libraries(tests wsock32.lib ws2_32.lib)
else() # We are under UNIX
    target_link_options(tests PRIVATE -pthread)
endif()

# The next two lines enable CMake’s test runner to discover the tests included in the binary,
# using the GoogleTest CMake module.
include(GoogleTest)
gtest_discover_tests(tests)