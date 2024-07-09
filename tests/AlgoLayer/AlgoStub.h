//
// Created by simatic on 04/04/202424.
//

#ifndef FBAE_ALGO_STUB_H
#define FBAE_ALGO_STUB_H

#include "AlgoLayer/AlgoLayer.h"
#include "InitDoneSupervisor.h"

class AlgoStub : public AlgoLayer {
 public:
  AlgoStub(std::unique_ptr<CommLayer> commLayer,
           InitDoneSupervisor &initDoneSupervisor);

  void callbackReceive(std::string &&algoMsgAsString) override;
  void callbackInitDone() override;
  void execute() override;
  void totalOrderBroadcast(
      const fbae_SessionLayer::SessionMsg &sessionMsg) override;
  void terminate() override;
  [[nodiscard]] std::string toString() override;

  [[nodiscard]] std::vector<std::string> &getReceived();

 private:
  InitDoneSupervisor &initDoneSupervisor;
  std::vector<std::string> received;
};

#endif  // FBAE_ALGO_STUB_H
