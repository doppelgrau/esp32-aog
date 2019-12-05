#ifndef hwSetup_HPP
#define hwSetup_HPP
#include <Preferences.h>

  void hwSetupNodeMcuCytronNmea();
  void hwSetupF9PIoBoardNmea();
  void hwSetupInitial();
  void hwSetupNetworkAp();
  void hwSetupWebSetup();
  void hwSetupWebNetwork();
  char* hwSetupHardwareIdToName(uint8_t setup);

  extern bool hwSetupHasEthernet;
#endif
