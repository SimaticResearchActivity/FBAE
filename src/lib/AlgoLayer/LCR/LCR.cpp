#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "LCR.h"
#include "LCRMessage.h"
#include "../../msgTemplates.h"

using namespace fbae_LCRAlgoLayer;

LCRLayer::ProcessData::ProcessData(rank_t processCount) :
        vectorClock(std::vector<uint32_t>(processCount)),
        pending(std::vector<StructBroadcastMessage>(0)) {

    // Initialize to zero all elements of the process's vector clock.
    // TODO: Does the vector constructor initializes the memory of each element?
    for (rank_t i = 0; i < processCount; i++)
        this->vectorClock[i] = 0;
}

LCRLayer::LCRLayer(std::unique_ptr<CommLayer> commLayer)
        :  processData(), AlgoLayer(std::move(commLayer)) {

    // Get the process count.
    const uint32_t processCount = getSessionLayer()->getArguments().getSites().size();

    // Call the constructor on each process data block by specifying the amount of
    // processes in the algorithm.
    for (uint32_t i = 0; i < processCount; i++)
        processData.emplace_back(processCount);
}

void LCRLayer::callbackReceive(std::string &&algoMsgAsString) {
    // Get the total process count.
    const uint32_t processCount = getSessionLayer()->getArguments().getSites().size();

    // Deserialize the message.
    auto message = deserializeStruct<StructBroadcastMessage>(std::move(algoMsgAsString));

    // Get the rank of the current and next processes.
    const rank_t currentProcessRank = message.currentRank;
    const rank_t nextProcessRank = (currentProcessRank + 1) % processCount;

    // Differentiate between if the message is being delivered or being acknowledged.
    switch (message.messageId) {
        case MessageId::Message: {
            // Get the sender's time on the message and the current process's clocks.
            const uint32_t messageClock = message.vectorClock[message.senderRank];
            const uint32_t currentClock = processData[currentProcessRank].vectorClock[message.senderRank];

            // TODO: I did not understand what this was supposed to do.
            if (messageClock <= currentClock)
                return;

            // If the next process is the same as the one who initiated the message,
            // then all processes have had the message...
            const bool cycleFinished = nextProcessRank == message.senderRank;
            // ... and if so, the message becomes stable.
            message.isStable = cycleFinished;

            // Increment the clock of the current process.
            processData[currentProcessRank].vectorClock[message.senderRank] += 1;
            // Append the message to the list of messages of the current process.
            processData[currentProcessRank].pending.push_back(message);

            // If the cycle is finished, the message should become an acknowledgment message.
            // if not, it should stay as-is.
            message.messageId = cycleFinished ? MessageId::Acknowledgement : MessageId::Message;

            break;
        }
        case MessageId::Acknowledgement: {
            // Get the rank of the process after the next process, and check whether it
            // is equal to the sender, if so abort.
            const rank_t nextNextProcessRank = (nextProcessRank + 1) % processCount;
            if (nextNextProcessRank == message.senderRank)
                return;

            // Iterate through all pending messages of the current process and
            // if the vector clocks are aligned, mark them as stable.
            for (auto pendingMessage : processData[currentProcessRank].pending)
                if (pendingMessage.vectorClock == message.vectorClock)
                    pendingMessage.isStable = true;

            break;
        }
        default: {
            std::cerr << "ERROR\tLCRAlgoLayer: Unexpected messageId (" << static_cast<int>(message.messageId) << ")\n";
            exit(EXIT_FAILURE);
        }
    }

    // The message must then pass to the next process.
    message.currentRank = nextProcessRank;

    // Serialize the process and send int to the next process.
    const auto serialized = serializeStruct<StructBroadcastMessage>(message);
    getCommLayer().send(nextProcessRank, serialized);

    // Try to deliver the message (if we should).
    if (message.messageId == MessageId::Acknowledgement) {
        while (processData[currentProcessRank].pending[0].isStable) {
            // TODO, I don't know what to do here.
            std::cout << "We delivered a message!\n";
            processData.erase(processData.begin());
        }
    }
}

void LCRLayer::execute() {
    std::vector<rank_t> broadcast_sites(getSessionLayer()->getArguments().getSites().size());
    std::iota(broadcast_sites.begin(), broadcast_sites.end(), 0);

    setBroadcastersGroup(std::move(broadcast_sites));
}

void LCRLayer::terminate() {
    getCommLayer()->terminate();
}

std::string LCRLayer::toString() {
    return "LCR";
}

void LCRLayer::totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMessage) {
    // Get the number of processes.
    const uint32_t processCount = getSessionLayer()->getArguments().getSites().size();

    // Get the rank of the sender and the receiver (which is the sender's successor).
    const rank_t sender = 0;
    const rank_t receiver = (sender + 1) % processCount;

    // Increase the vector clock of the sender process (broadcasting is an action).
    processData[sender].vectorClock[sender] += 1;

    // Construct the message struct.
    const StructBroadcastMessage message = {
            .messageId = MessageId::Message,
            .senderRank = sender,
            .currentRank = receiver,
            .isStable = false,
            .vectorClock = processData[sender].vectorClock,
            .sessionMessage = sessionMessage,
    };

    // Append the message to the list of pending messages to be delivered by the
    // sender process.
    processData[sender].pending.push_back(message);

    // Serialize the message...
    auto serialized = serializeStruct<StructBroadcastMessage>(message);
    // ... and send it to its successor.
    getCommLayer()->send(receiver, serialized);
}
