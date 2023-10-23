//
// Created by lardeur on 09/10/23.
//

#ifndef FBAE_BBOBBALGOLAYER_H
#define FBAE_BBOBBALGOLAYER_H


#include "AlgoLayer.h"

class BBOBBAlgoLayer : public AlgoLayer {
private :
    std::vector<std::unique_ptr<CommPeer>> peers; //peers that the entity will communicate with
public :
    void callbackHandleMessageAsHost(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    void callbackHandleMessageAsNonHostPeer(std::unique_ptr<CommPeer> peer, const std::string &msgString) override;
    bool executeAndProducedStatistics() override;
    void totalOrderBroadcast(const std::string &msg) override;
    void terminate() override;
    std::string toString() override;
};

enum class BroadcasterMsgId : unsigned char
{
    DisconnectIntent = 97, // We start with a value which be displayed as a character in debugger + the enum values are different from values in enum SequencerMsgId
    MessageToBroadcast,
    RankInfo,
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



#endif //FBAE_BBOBBALGOLAYER_H
