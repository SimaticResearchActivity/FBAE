add_executable(fbae
  main.cpp
  options.cpp options.h OptParserExtended.cpp OptParserExtended.h
        Measures.cpp Measures.h
        Param.cpp Param.h
        CommLayer/CommLayer.cpp
        CommLayer/CommLayer.h
        CommLayer/EnetCommLayer/EnetCommLayer.cpp
        CommLayer/EnetCommLayer/EnetCommLayer.h
        AlgoLayer/AlgoLayer.cpp
        AlgoLayer/AlgoLayer.h
        AlgoLayer/SequencerAlgoLayer/SequencerAlgoLayer.cpp
        AlgoLayer/SequencerAlgoLayer/SequencerAlgoLayer.h
        SessionLayer/SessionLayer.cpp
        SessionLayer/SessionLayer.h
        CommLayer/CommPeer.h
        CommLayer/EnetCommLayer/EnetCommPeer.cpp
        CommLayer/EnetCommLayer/EnetCommPeer.h
        msgTemplates.h
        AlgoLayer/SequencerAlgoLayer/SequencerAlgoLayerMsg.h
        SessionLayer/SessionLayerMsg.h
        AlgoLayer/BBOBBAlgoLayer/BBOBBAlgoLayer.cpp
        AlgoLayer/BBOBBAlgoLayer/BBOBBAlgoLayer.h
        AlgoLayer/BBOBBAlgoLayer/BBOBBAlgoLayerMsg.h
        AlgoLayer/BBOBBAlgoLayer/BBOBBAlgoLayerMsg.h
        SessionLayer/SessionLayerMsg.h
        CommLayer/TcpCommLayer/TcpCommLayer.cpp
        CommLayer/TcpCommLayer/TcpCommLayer.h
        CommLayer/TcpCommLayer/TcpCommPeer.cpp
        CommLayer/TcpCommLayer/TcpCommPeer.h
        basicTypes.h
        get_cpu_time.cpp
        get_cpu_time.h)
target_link_libraries(fbae PUBLIC enet)
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