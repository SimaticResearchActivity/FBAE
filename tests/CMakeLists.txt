add_executable(serializationOverhead
        mainSerializationOverhead.cpp
        ../src/AlgoLayer/BBOBB/BBOBBMsg.h
        ../src/AlgoLayer/Sequencer/SequencerMsg.h
        ../src/SessionLayer/SessionLayerMsg.h
)
target_link_libraries(serializationOverhead)
