#ifndef hwSetup_HPP
#define hwSetup_HPP
#include <Preferences.h>

  void hwSetupWifiApOnly();
  void hwSetupNodeMcuNmea();
  void hwSetupF9PIoBoardNmea();
  void hwSetupNetworkAp(Preferences preferences);
  void hwSetupWebSetup();
  char* hwSetupIdToName(uint8_t setup);
#endif
