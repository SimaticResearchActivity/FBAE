add_executable(fbae
  main.cpp
  options.cpp options.h OptParserExtended.cpp OptParserExtended.h
        Measures.cpp Measures.h
        Arguments.cpp Arguments.h
        CommLayer/CommLayer.cpp
        CommLayer/CommLayer.h
        AlgoLayer/AlgoLayer.cpp
        AlgoLayer/AlgoLayer.h
        AlgoLayer/Sequencer/Sequencer.cpp
        AlgoLayer/Sequencer/Sequencer.h
        SessionLayer/SessionLayer.cpp
        SessionLayer/SessionLayer.h
        msgTemplates.h
        AlgoLayer/Sequencer/SequencerMsg.h
        SessionLayer/SessionLayerMsg.h
        AlgoLayer/BBOBB/BBOBB.cpp
        AlgoLayer/BBOBB/BBOBB.h
        AlgoLayer/BBOBB/BBOBBMsg.h
        AlgoLayer/BBOBB/BBOBBMsg.h
        SessionLayer/SessionLayerMsg.h
        CommLayer/Tcp/Tcp.cpp
        CommLayer/Tcp/Tcp.h
        basicTypes.h
        get_cpu_time.cpp
        get_cpu_time.h
        SessionLayer/Session/Session.cpp
        SessionLayer/Session/Session.h)
if (WIN32)
  target_link_libraries(fbae PUBLIC ws2_32.lib winmm.lib)
else() # We are under UNIX
  target_link_options(fbae PUBLIC -pthread)
endif()

add_custom_target(copy-resources ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Resources)
file(GLOB RESOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/Resources/*.*)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Resources
        DEPENDS ${CMAKE_SOURCE_DIR}/Resources
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Resources
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${RESOURCES}
        ${CMAKE_CURRENT_BINARY_DIR}/Resources)
add_dependencies(fbae copy-resources)