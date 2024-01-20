#ifndef FBAE_BBOBBALGOLAYERMSG_H
#define FBAE_BBOBBALGOLAYERMSG_H

#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"

namespace fbae_BBOBBAlgoLayer {

    enum class MsgId : unsigned char {
        RankInfo,
        AckDisconnectIntent,
        Step,
        MessageToReceive,
        DisconnectIntent,
    };

    struct RankInfoMessage {
        MsgId msgId{};
        unsigned char senderRank{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId, senderRank); // serialize things by passing them to the archive
        }
    };

    using BroadcasterRankInfo = RankInfoMessage;
    using BroadcasterDisconnectIntent = RankInfoMessage;

    struct StructGenericMsgWithoutData {
        MsgId msgId{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId); // serialize things by passing them to the archive
        }
    };

    using StructAckDisconnectIntent = StructGenericMsgWithoutData;

    struct StepMessage {
        MsgId msgId{};
        unsigned char senderRank{};
        int stepNumber;
        int wave;
        std::vector<std::string> msgBroadcasted;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId, senderRank, stepNumber, wave, msgBroadcasted); // serialize things by passing them to the archive
        }
    };

    struct Message {
        MsgId msgId{};
        unsigned char senderRank{};
        std::string sessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId, senderRank, sessionMsg); // serialize things by passing them to the archive
        }
    };



}

#endif //FBAE_BBOBBALGOLAYERMSG_H
