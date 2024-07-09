//
// Created by simatic on 05/04/202424.
//

#include "InitDoneSupervisor.h"

void InitDoneSupervisor::callbackInitDone() { waitCallbackInitDone.release(); }

void InitDoneSupervisor::waitInitDone() { waitCallbackInitDone.acquire(); }
