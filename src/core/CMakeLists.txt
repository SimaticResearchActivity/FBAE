

add_library(lib_fbae
        Arguments.cpp
	Arguments.h
	OptParserExtended.cpp
	OptParserExtended.h
        adaptCereal.h
        basicTypes.h
        msgTemplates.h
        options.cpp
	options.h
        AlgoLayer/AlgoLayer.cpp
        AlgoLayer/AlgoLayer.h
        AlgoLayer/AlgoLayerMsg.h
		AlgoLayer/AlgoStub.cpp
		AlgoLayer/AlgoStub.h
        AlgoLayer/BBOBB/BBOBB.cpp
        AlgoLayer/BBOBB/BBOBB.h
        AlgoLayer/BBOBB/BBOBBMsg.h
		AlgoLayer/InitDoneSupervisor.cpp
		AlgoLayer/InitDoneSupervisor.h
        AlgoLayer/Sequencer/Sequencer.cpp
        AlgoLayer/Sequencer/Sequencer.h
        AlgoLayer/Sequencer/SequencerMsg.h
		AlgoLayer/Trains/Trains.cpp
		AlgoLayer/Trains/Trains.h
        CommLayer/CommLayer.cpp
        CommLayer/CommLayer.h
		CommLayer/CommStub.cpp
		CommLayer/CommStub.h
		CommLayer/Tcp/Tcp.cpp
		CommLayer/Tcp/Tcp.h
        SessionLayer/SessionLayer.cpp
        SessionLayer/SessionLayer.h
		SessionLayer/SessionLayerMsg.cpp
        SessionLayer/SessionLayerMsg.h
		SessionLayer/SessionStub.cpp
		SessionLayer/SessionStub.h
        SessionLayer/PerfMeasures/Measures.cpp
	    SessionLayer/PerfMeasures/Measures.h
        SessionLayer/PerfMeasures/PerfMeasures.cpp
        SessionLayer/PerfMeasures/PerfMeasures.h
        SessionLayer/PerfMeasures/get_cpu_time.cpp
        SessionLayer/PerfMeasures/get_cpu_time.h
        AlgoLayer/LCR/LCR.cpp
        AlgoLayer/LCR/LCR.h
		AlgoLayer/LCR/LCRMessage.h
        AlgoLayer/LCR/LCRTypedefs.h
        Logger/LoggerConfig.h
        Logger/LoggerConfig.cpp)

target_include_directories(lib_fbae PUBLIC ./)
target_link_libraries(lib_fbae PUBLIC boost_subset_for_asio Yoctolib usb-1.0 PRIVATE log4cxx )
set_target_properties(Yoctolib PROPERTIES
		IMPORTED_LOCATION ${YOCTO_LIBRARY_PATH}
)
