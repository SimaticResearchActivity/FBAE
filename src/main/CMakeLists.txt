add_executable(fbae 
        main.cpp
)

target_link_libraries(fbae lib_fbae)
if (WIN32)
  target_link_libraries(fbae wsock32.lib ws2_32.lib)
else() # We are under UNIX
  target_link_options(fbae PUBLIC -pthread)
endif()
