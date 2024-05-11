#pragma once

#include "../AlgoLayer.h"

class Trains: public AlgoLayer {
public:
    explicit Trains(std::unique_ptr<CommLayer> commLayer) noexcept;

    void callbackReceive(std::string && serializedMessagePacket) noexcept override;
    void execute() noexcept override;
    void terminate() noexcept override;
    void totalOrderBroadcast(const fbae_SessionLayer::SessionMsg &sessionMsg) noexcept override;

    std::string toString() noexcept override;

private:
    // Mettre tout ce dont on a besoin : fonctions + paramètres de notre classe


    // Comment on modélise le train, enfin où on met notre train ?
    /**
     * @brief List containing the messages (the train).
     */
    std::vector<fbae_TrainsAlgoLayer::MessagePacket> train;

    /**
     * @brief Internal function used to send the train to the successor.
     */
    void deliverTrain() noexcept;

    /**@brief Internal function used to get all the messages from
     * the train.
     */
     void getMessages() noexcept;

    /**
     * @brief Internal function used to delete the message of the
     * successor from the train.
     */
    void deleteMessage() noexcept;

    /**
     * @brief Internal function used to add a message to the train.
     */
     void addMessage() noexcept;

};

