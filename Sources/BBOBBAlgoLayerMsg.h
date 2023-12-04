//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBBALGOLAYERMSG_H
#define FBAE_BBOBBALGOLAYERMSG_H

namespace fbae_BBOBBAlgoLayer {

    enum class MsgId : unsigned char
    {
        RankInfo,
        AllBroadcastersConnected,
        AckDisconnectIntent,
        Step,
        ReceiveMessage,
        DisconnectIntent,
    };

    struct RankInfoMessage
    {
        MsgId msgId{};
        unsigned char  senderRank{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank); // serialize things by passing them to the archive
        }
    };

    using BroadcasterRankInfo = RankInfoMessage;
    using BroadcasterDisconnectIntent = RankInfoMessage;

    struct StructGenericMsgWithoutData
    {
        MsgId msgId{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId); // serialize things by passing them to the archive
        }
    };

    using StructAckDisconnectIntent = StructGenericMsgWithoutData;
    using StructAllBroadcastersConnected = StructGenericMsgWithoutData;

    struct BroadcasterMessageToSend
    {
        MsgId msgId{};
        unsigned char senderRank{};
        std::string sessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank, sessionMsg); // serialize things by passing them to the archive
        }
    };

    struct BBOBBSendMessage
    {
        MsgId msgId{};
        unsigned char senderRank{};
        int seqNum{};
        std::string sessionMsg;

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId, senderRank, seqNum, sessionMsg); // serialize things by passing them to the archive
        }
    };

}

#endif //FBAE_BBOBBALGOLAYERMSG_H
