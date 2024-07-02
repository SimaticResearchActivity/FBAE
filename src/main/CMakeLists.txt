add_executable(fbae 
        main.cpp
)

target_link_libraries(fbae lib_fbae log4cxx)

if (WIN32)
  target_link_libraries(fbae ws2_32.lib winmm.lib)
else() # We are under UNIX
  target_link_options(fbae PUBLIC -pthread)
endif()