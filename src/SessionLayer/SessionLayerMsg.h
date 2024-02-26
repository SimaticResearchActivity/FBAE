#pragma once

#include "../adaptCereal.h"
#include "cereal/types/polymorphic.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/chrono.hpp"
#include "cereal/types/string.hpp"
#include "../basicTypes.h"

/**
 * @brief Namespace for SessionLayer. It should be called fbae_SessionLayer, but there is an encoding
 * bug related to Cereal ==> For the moment, we use short names.
 */
namespace fbaeSL {
    //---------------------------------------------------
    // Messages totalOrderBroadcast between any instances of SessionLayer (or subclasses)
    //---------------------------------------------------
    /**
     * @brief Message Id used within @SessionLayer.
    */
    enum class SessionMsgId : MsgId_t
    {
        // Messages used within PerfMeasures subclass
        FinishedPerfMeasures, /// Broadcast message signifying its sender has finished to send all of its @PerfMeasure messages.
        FirstBroadcast, /// Broadcast message used by a sender when it broadcasts a message for the first time.
        PerfMeasure, /// Broadcast message used by a sender as Ping message
        PerfResponse, /// Broadcast message used as a Pong message in response to a @PerfMeasure message
        // Messages used for serialization tests
        TestMessage /// Broadcast message used for tests
    };

    /**
     * @brief Session Base class
     */
     struct SessionBaseClass
     {
         SessionBaseClass() = default;
         explicit SessionBaseClass(SessionMsgId msgId) : msgId{msgId} {}
         virtual ~SessionBaseClass() = default;

         SessionMsgId msgId{};

         // This method lets cereal know which data members to serialize
         template<class Archive>
         void serialize(Archive& archive)
         {
             archive(msgId);
         }
     };

    /**
     * @brief Generic message structure containing only a msgId.
     */
    struct GenericSessionMsgWithId : SessionBaseClass
    {
        GenericSessionMsgWithId() = default;
        explicit GenericSessionMsgWithId(SessionMsgId msgId)
        : SessionBaseClass{msgId}
        {}
        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive( cereal::base_class<SessionBaseClass>(this) );
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
     * @brief Structure of @PerfMeasure and @PerfResponse messages. Note: This structure should be called
     * SessionPerf, but there is an encoding bug related to Cereal ==> For the moment, we use short names.
     */
    struct SP : SessionBaseClass
    {
        SP() = default;
        SP(SessionMsgId msgId, rank_t senderPos, int32_t msgNum, std::chrono::time_point<std::chrono::system_clock> sendTime, std::string filler)
        : SessionBaseClass{msgId}
        , senderPos{senderPos}
        , msgNum{msgNum}
        , sendTime{sendTime}
        , filler{std::move(filler)}
        {}

        rank_t senderPos{};
        int32_t msgNum{};
        std::chrono::time_point<std::chrono::system_clock> sendTime;
        std::string filler;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::base_class<SessionBaseClass>(this), senderPos, msgNum, sendTime, filler);
        }
    };

    /**
     * @brief Structure of messages used for tests? Note: This structure should be called
     * SessionTest, but there is an encoding bug related to Cereal ==> For the moment, we use short names.
     */
    struct ST : SessionBaseClass
    {
        ST() = default;
        ST(SessionMsgId msgId, std::string payload)
        : SessionBaseClass{msgId}
        , payload{std::move(payload)}
        {}
        std::string payload;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::base_class<SessionBaseClass>(this), payload);
        }
    };

    /**
     * @brief Shortcut for std::shared_ptr<fbae_SessionLayer::SessionBase>
     */
    using SessionMsg = std::shared_ptr<fbaeSL::SessionBaseClass>;
}

// Cereal polymorphic registration must be done outside fbae_SessionLayer namespace scope
CEREAL_REGISTER_TYPE(fbaeSL::GenericSessionMsgWithId);
CEREAL_REGISTER_TYPE(fbaeSL::SP);
CEREAL_REGISTER_TYPE(fbaeSL::ST);

// We do not need to call CEREAL_REGISTER_POLYMORPHIC_RELATION() macro because
// we call cereal::base_class in the different subclasses (see documentation of
// CEREAL_REGISTER_POLYMORPHIC_RELATION at
// http://uscilab.github.io/cereal/assets/doxygen/polymorphic_8hpp.html#a5d730928a52a379ad0d6f2dcdee07953)