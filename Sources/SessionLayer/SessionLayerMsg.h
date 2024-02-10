#pragma once

#include "cereal/archives/binary.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/string.hpp"
#include "../basicTypes.h"

namespace fbae_SessionLayer {
    //---------------------------------------------------
// Messages totalOrderBroadcast by SessionLayer (to SessionLayer)
//---------------------------------------------------
    /**
     * @brief Message Id used within @SessionLayer.
    */
    enum class SessionMsgId : MsgId_t
    {
        FinishedPerfMeasures, /// Broadcast message signifying its sender has finished to send all of its @PerfMeasure messages.
        FirstBroadcast, /// Broadcast message used by a sender when it broadcasts a message for the first time.
        PerfMeasure, /// Broadcast message used by a sender as Ping message
        PerfResponse /// Broadcast message used as a Pong message in response to a @PerfMeasure message
    };

    /**
     * @brief Generic message structure containing only a msgId.
     */
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

    /**
     * @brief Structure of @FinishedPerfMeasures message.
     */
    using SessionFinishedPerfMeasures = GenericSessionMsgWithId;

    /**
     * @brief Structure of @FirstBroadcast message.
     */
    using SessionFirstBroadcast = GenericSessionMsgWithId;

    /**
     * @brief Structure of @PerfMeasure message.
     */
    struct SessionPerfMeasure
    {
        SessionMsgId msgId{};
        rank_t senderRank{};
        int32_t msgNum{};
        std::chrono::time_point<std::chrono::system_clock> sendTime;
        std::string filler;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank, msgNum, sendTime, filler); // serialize things by passing them to the archive
        }
    };

    /**
     * @brief Structure of @PerfResponse message.
     */
    struct SessionPerfResponse
    {
        SessionMsgId msgId{};
        rank_t perfMeasureSenderRank{};
        int32_t perfMeasureMsgNum{};
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