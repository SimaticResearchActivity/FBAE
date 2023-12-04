add_executable(fbae
  main.cpp
  options.cpp options.h OptParserExtended.cpp OptParserExtended.h
        Measures.cpp Measures.h
        Param.cpp Param.h
        CommLayer.cpp
        CommLayer.h
        EnetCommLayer.cpp
        EnetCommLayer.h
        AlgoLayer.cpp
        AlgoLayer.h
        SequencerAlgoLayer.cpp
        SequencerAlgoLayer.h
        SessionLayer.cpp
        SessionLayer.h
        CommPeer.h
        EnetCommPeer.cpp
        EnetCommPeer.h
        msgTemplates.h
        SequencerAlgoLayerMsg.h
        SessionLayerMsg.h
        TcpCommLayer.cpp
        TcpCommLayer.h
        TcpCommPeer.cpp
        TcpCommPeer.h)
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