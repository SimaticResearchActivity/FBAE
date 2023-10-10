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
        BBOBBAlgoLayer.cpp
        BBOBBAlgoLayer.h
        BBOBBAlgoLayerMsg.h
        BBOBBAlgoLayerMsg.h)
target_link_libraries(fbae PUBLIC enet)
if (WIN32)
  target_link_libraries(fbae PUBLIC ws2_32.lib winmm.lib)
else() # We are under UNIX
  target_link_options(fbae PUBLIC -pthread)
endif()
