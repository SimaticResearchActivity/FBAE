#pragma once

#include "../adaptCereal.h"
#include "cereal/types/polymorphic.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/chrono.hpp"
#include "../basicTypes.h"

/**
 * @brief Namespace for SessionLayer.
 */
namespace fbae_SessionLayer {
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

         SessionMsgId msgId{0};

         /**
          * @brief Method to facilitate unit tests where it is required to get payload field of @Sessiontest messages. It should never be calles for other types od Session messages.
          * @return "SessionBaseClass::getPayload() should never be called"
          */
         virtual std::string getPayload();

     private:
         friend class cereal::access;
         // This method lets cereal know which data members to serialize
         template<class Archive>
         void serialize(Archive& archive)
         {
             archive(msgId);
         }
     };

    /**
     * @brief Structure of @FinishedPerfMeasures message.
     */
    using SessionFinishedPerfMeasures = SessionBaseClass;

    /**
     * @brief Structure of @FirstBroadcast message.
     */
    using SessionFirstBroadcast = SessionBaseClass;

    /**
     * @brief Structure of @PerfMeasure and @PerfResponse messages.
     */
    struct SessionPerf : SessionBaseClass
    {
        SessionPerf(SessionMsgId msgId, rank_t senderPos, int32_t msgNum, std::chrono::time_point<std::chrono::system_clock> sendTime, std::string filler)
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

    private:
        friend class cereal::access;
        SessionPerf() = default;
        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::base_class<SessionBaseClass>(this), senderPos, msgNum, sendTime, filler);
        }
    };

    /**
     * @brief Structure of messages used for tests.
     */
    struct SessionTest : SessionBaseClass
    {
        SessionTest(SessionMsgId msgId, std::string payload)
        : SessionBaseClass{msgId}
        , payload{std::move(payload)}
        {}

        /**
         * @brief Accessor to @payload field
         * @return @payload
         */
        std::string getPayload() override;

    private:
        std::string payload;
        friend class cereal::access;
        SessionTest() = default;
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
    using SessionMsg = std::shared_ptr<fbae_SessionLayer::SessionBaseClass>;
}

// Cereal polymorphic registration must be done outside fbae_SessionLayer namespace scope
// We register all of these types with very short names, so that the full names (including the namespace) is not used
// when the name is stored in serialized data. Note: The name is stored in serialized data as soon as the pointer used
// is defined as a pointer to the base class and not the pointed subclass.
CEREAL_REGISTER_TYPE_WITH_NAME(fbae_SessionLayer::SessionBaseClass, "A")
CEREAL_REGISTER_TYPE_WITH_NAME(fbae_SessionLayer::SessionPerf, "B")
CEREAL_REGISTER_TYPE_WITH_NAME(fbae_SessionLayer::SessionTest, "C")

// We do not need to call CEREAL_REGISTER_POLYMORPHIC_RELATION() macro because
// we call cereal::base_class in the different subclasses (see documentation of
// CEREAL_REGISTER_POLYMORPHIC_RELATION at
// http://uscilab.github.io/cereal/assets/doxygen/polymorphic_8hpp.html#a5d730928a52a379ad0d6f2dcdee07953)