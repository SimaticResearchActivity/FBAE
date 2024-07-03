add_executable(fbae 
        main.cpp
)

target_link_libraries(fbae lib_fbae log4cxx)

if (WIN32)
  target_link_libraries(fbae ws2_32.lib winmm.lib)
else() # We are under UNIX
  target_link_options(fbae PUBLIC -pthread)
endif()

add_custom_target(copy-res ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/res)
file(GLOB RESOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/res/*.*)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/res
                   DEPENDS ${CMAKE_SOURCE_DIR}/res
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           ${RESOURCES}
                           ${CMAKE_CURRENT_BINARY_DIR}/res)
add_dependencies(fbae copy-res)