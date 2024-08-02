cmake_minimum_required(VERSION 3.28)
project(Test_POWERAPI)

set(CMAKE_CXX_STANDARD 20)

add_executable(Test_POWERAPI main.cpp)

target_include_directories(Test_POWERAPI PRIVATE
        /usr/local/include/bsoncxx/v_noabi
        /usr/local/include/mongocxx/v_noabi
        /usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc
)

target_link_libraries(Test_POWERAPI
        /usr/local/lib/libbsoncxx.so
        /usr/local/lib/libmongocxx.so
)