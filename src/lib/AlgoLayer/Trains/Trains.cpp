#include <iostream>
#include <numeric>
#include "../../SessionLayer/SessionLayer.h"
#include "Trains.h"
#include "TrainsMessage.h"
#include "../../msgTemplates.h"

#include "Logger/Logger.h"

using namespace fbae_LCRAlgoLayer;

//Comment transmettre le train à la personne suivante ?
void Trains::deliverTrain() noexcept {

}

void Trains::getMessages() noexcept {
    for (const auto& tuple : train) {
        std::cout << "ID: " << std::get<0>(tuple) << ", Message: " << std::get<1>(tuple) << std::endl;
    }
}

// Comment on récupère le rang d'un mec ?
void Trains::deleteMessage() noexcept {
    // Itérateur pour parcourir la liste
    auto it = train.begin();

    // Parcourir la liste
    while (it != train.end() && std::get<0>(*it) == message.senderRank +1) {
        // Supprimer le tuple courant s'il a été envoyé par le successeur.
        it = train.erase(it);
    }
}

//Comment on récupère le message qu'un mec veut envoyer ?
void Trains::addMessage() noexcept {
    // Création d'un nouveau tuple avec les valeurs spécifiées
    fbae_TrainsAlgoLayer::MessagePacket newTuple = std::make_tuple(id, message);

    // Ajout du message au train
    train.push_back(newTuple);
}