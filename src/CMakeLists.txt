add_executable(fbae
  main.cpp
  options.cpp options.h OptParserExtended.cpp OptParserExtended.h
        SessionLayer/PerfMeasures/Measures.cpp SessionLayer/PerfMeasures/Measures.h
        Arguments.cpp Arguments.h
        CommLayer/CommLayer.cpp
        CommLayer/CommLayer.h
        AlgoLayer/AlgoLayer.cpp
        AlgoLayer/AlgoLayer.h
        AlgoLayer/AlgoLayerMsg.h
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
        SessionLayer/PerfMeasures/get_cpu_time.cpp
        SessionLayer/PerfMeasures/get_cpu_time.h
        SessionLayer/PerfMeasures/PerfMeasures.cpp
        SessionLayer/PerfMeasures/PerfMeasures.h
        AlgoLayer/AlgoLayerMsg.h
        adaptCereal.h
)
if (WIN32)
  target_link_libraries(fbae PUBLIC ws2_32.lib winmm.lib)
else() # We are under UNIX
  target_link_options(fbae PUBLIC -pthread)
endif()

add_custom_target(copy-resources ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/res)
file(GLOB RESOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/res/*.*)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/res
        DEPENDS ${CMAKE_SOURCE_DIR}/res
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${RESOURCES}
        ${CMAKE_CURRENT_BINARY_DIR}/res)
add_dependencies(fbae copy-resources)