//
// Created by simatic on 2/17/24.
//

#ifndef FBAE_SESSION_STUB_H
#define FBAE_SESSION_STUB_H

#include "AlgoLayer/AlgoLayer.h"
#include "SessionLayer/SessionLayer.h"

namespace fbae::core::SessionLayer {

class SessionStub : public fbae::core::SessionLayer::SessionLayer {
 public:
  SessionStub(const fbae::core::Arguments &arguments, fbae::core::rank_t rank,
              std::unique_ptr<fbae::core::AlgoLayer::AlgoLayer> algoLayer);

  void callbackDeliver(fbae::core::rank_t senderPos,
                       fbae::core::SessionLayer::SessionMsg msg) override;
  void callbackInitDone() override;
  void execute() override;
  [[nodiscard]] std::vector<std::pair<fbae::core::rank_t, fbae::core::SessionLayer::SessionMsg>>
      &getDelivered();
  [[nodiscard]] bool isCallbackInitDoneCalled() const;

 private:
  /**
   * @brief Vector of [sender,msg] for which @callbackDeliver() method was
   * called
   */
  std::vector<std::pair<fbae::core::rank_t, fbae::core::SessionLayer::SessionMsg>> delivered;

  /**
   * @brief Indicates whether @SessionLayer::callbackInitDone called or not.
   */
  bool callbackInitDoneCalled{false};
};

}  // namespace fbae::core::SessionLayer

#endif  // FBAE_SESSION_STUB_H
