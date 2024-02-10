add_executable(serializationOverhead
        mainSerializationOverhead.cpp
        ../Sources/AlgoLayer/BBOBB/BBOBBMsg.h
        ../Sources/AlgoLayer/Sequencer/SequencerMsg.h
        ../Sources/SessionLayer/SessionLayerMsg.h
)
target_link_libraries(serializationOverhead)
