//
// Created by simatic on 04/04/202424.
//

#ifndef FBAE_ALGO_STUB_H
#define FBAE_ALGO_STUB_H

#include "AlgoLayer/AlgoLayer.h"
#include "InitDoneSupervisor.h"

namespace fbae::core::AlgoLayer {

class AlgoStub : public fbae::core::AlgoLayer::AlgoLayer {
 public:
  AlgoStub(std::unique_ptr<fbae::core::CommLayer::CommLayer> commLayer,
           InitDoneSupervisor &initDoneSupervisor);

  void callbackReceive(std::string &&algoMsgAsString) override;
  void callbackInitDone() override;
  void execute() override;
  void totalOrderBroadcast(
      const fbae::core::SessionLayer::SessionMsg &sessionMsg) override;
  void terminate() override;
  [[nodiscard]] std::string toString() override;

  [[nodiscard]] std::vector<std::string> &getReceived();

 private:
  InitDoneSupervisor &initDoneSupervisor;
  std::vector<std::string> received;
};

}  // namespace fbae::core::AlgoLayer

#endif  // FBAE_ALGO_STUB_H
