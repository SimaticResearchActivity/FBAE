add_executable(fbae 
        main.cpp
)

target_link_libraries(fbae lib_fbae log4cxx)

if (WIN32)
  target_link_libraries(fbae wsock32.lib ws2_32.lib)
else() # We are under UNIX
  target_link_options(fbae PUBLIC -pthread)
endif()

add_custom_target(copy-res-fbae ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)
add_dependencies(fbae copy-res-fbae)