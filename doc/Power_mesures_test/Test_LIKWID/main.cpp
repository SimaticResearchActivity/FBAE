#include <cmath>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <bits/atomic_base.h>

#include "likwid.h"

using namespace std;

int main() {
  cout << "Hello, World!" << endl;

  // Init the topology and informations of CPUs 
  topology_init();
  CpuTopology_t const& topo = get_cpuTopology();
  CpuInfo_t const& cpuInfo = get_cpuInfo();

  cout  << "CPU :\n"
        //<< "Name: " << cpuInfo->name
        //<< "In short: " << cpuInfo->short_name << "\n"
        << "Osname: " << cpuInfo->osname << "\n"
        << "Freq: " << cpuInfo->clock << "\n"
        << "Family: " << cpuInfo->family << "\n"
        << "Model: " << cpuInfo->model << "\n"
        << "Part: " << cpuInfo->part << endl;

  // Check if the CPU is supported and init the power measurement
  for (int cpuId = 0; cpuId < topo->numSockets; cpuId++) {
    if(power_init(cpuId) == 0) {
      cout << "CPU not supported" << endl;
      return -1;
    }
  }
  cout << "Init Done" << endl;

  // Measures
  auto powerDatas = vector<PowerData_t>(topo->numSockets);

  for (int cpuId = 0; cpuId < topo->numSockets; cpuId++) {
    power_start(powerDatas[cpuId], cpuId, PowerType::PKG);
  }
  cout << "Start Measure" << endl;

  sleep(2);

  cout << "End Sleep" << endl;
  
  for (int cpuId = 0; cpuId < topo->numSockets; cpuId++) {
    power_stop(powerDatas[cpuId], cpuId, PowerType::PKG);

    cout << "Stop Measure " << cpuId;

    power_printEnergy(powerDatas[cpuId]);
  }

  power_finalize();
  topology_finalize();

  cout << "Good Night, World!" << endl;

  return 0;
}
