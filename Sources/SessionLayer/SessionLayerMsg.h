#pragma once

#include "cereal/archives/binary.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/string.hpp"

namespace fbae_SessionLayer {
    //---------------------------------------------------
// Messages totalOrderBroadcast by SessionLayer (to SessionLayer)
//---------------------------------------------------
    enum class SessionMsgId : unsigned char
    {
        FinishedPerfMeasures = 48, // We start with a value which be displayed as a character in debugger
        FirstBroadcast,
        PerfMeasure,
        PerfResponse
    };

    struct GenericSessionMsgWithId
    {
        SessionMsgId msgId{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId); // serialize things by passing them to the archive
        }
    };

    using SessionFinishedPerfMeasures = GenericSessionMsgWithId;
    using SessionFirstBroadcast = GenericSessionMsgWithId;

    struct SessionPerfMeasure
    {
        SessionMsgId msgId{};
        unsigned char senderRank{};
        unsigned int msgNum{};
        std::chrono::time_point<std::chrono::system_clock> sendTime;
        std::string filler;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank, msgNum, sendTime, filler); // serialize things by passing them to the archive
        }
    };

    struct SessionPerfResponse
    {
        SessionMsgId msgId{};
        unsigned char perfMeasureSenderRank{};
        unsigned int perfMeasureMsgNum{};
        std::chrono::time_point<std::chrono::system_clock> perfMeasureSendTime;
        std::string perfMeasureFiller;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, perfMeasureSenderRank, perfMeasureMsgNum, perfMeasureSendTime, perfMeasureFiller); // serialize things by passing them to the archive
        }
    };

}
