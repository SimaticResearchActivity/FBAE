add_executable(serializationOverhead
        mainSerializationOverhead.cpp
        ../Sources/AlgoLayer/BBOBBAlgoLayer/BBOBBAlgoLayerMsg.h
        ../Sources/AlgoLayer/SequencerAlgoLayer/SequencerAlgoLayerMsg.h
        ../Sources/SessionLayer/SessionLayerMsg.h
)
target_link_libraries(serializationOverhead)
