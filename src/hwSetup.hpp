#ifndef hwSetup_HPP
#define hwSetup_HPP
#include <Preferences.h>

  void hwSetupWifiApOnly();
  void hwSetupNodeMcuCytronNmea();
  void hwSetupF9PIoBoardNmea();
  void hwSetupNetworkAp();
  void hwSetupWebSetup();
  char* hwSetupIdToName(uint8_t setup);
#endif
