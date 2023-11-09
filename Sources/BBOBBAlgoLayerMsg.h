//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBBALGOLAYERMSG_H
#define FBAE_BBOBBALGOLAYERMSG_H

namespace fbae_BBOBBAlgoLayer {

    enum class BroadcasterMsgId : unsigned char
    {
        RankInfo,
        AllBroadcastersConnected,
        AckDisconnectIntent,
        MessageToReceive,
        ReceiveMessage,
        DisconnectIntent,
    };

    struct RankInfoMessage
    {
        BroadcasterMsgId msgId{};
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

    struct GenericBroadcasterMsgWithoutData
    {
        BroadcasterMsgId msgId{};

        // This method lets cereal know which data members to serialize
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(msgId); // serialize things by passing them to the archive
        }
    };

    using AllBroadcastersConnected = GenericBroadcasterMsgWithoutData;
    using AckDisconnectIntent = GenericBroadcasterMsgWithoutData;






    struct BroadcasterMessageToSend
    {
        BroadcasterMsgId msgId{};
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
        BroadcasterMsgId msgId{};
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
