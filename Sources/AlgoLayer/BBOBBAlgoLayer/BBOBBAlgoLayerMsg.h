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
        DisconnectIntent
    };

    struct StructGenericMsgWithRank {
        MsgId msgId{};
        unsigned char senderRank{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId, senderRank); // serialize things by passing them to the archive
        }
    };

    using BroadcasterRankInfo = StructGenericMsgWithRank;
    using BroadcasterDisconnectIntent = StructGenericMsgWithRank;

    struct StructGenericMsgWithoutData {
        MsgId msgId{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId); // serialize things by passing them to the archive
        }
    };

    using StructAckDisconnectIntent = StructGenericMsgWithoutData;

    struct EncapsulationSessionMsg {
        unsigned char senderRank{};
        std::vector<std::string> sessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(senderRank, sessionMsg); // serialize things by passing them to the archive
        }
    };

    struct StepMsg {
        MsgId msgId{};
        unsigned char senderRank{};
        int wave;
        int step;
        std::vector<EncapsulationSessionMsg> msgBroadcast;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive &archive) {
            archive(msgId, senderRank, wave, step, msgBroadcast); // serialize things by passing them to the archive
        }
    };


}

#endif //FBAE_BBOBBALGOLAYERMSG_H
