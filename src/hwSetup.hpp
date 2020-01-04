#ifndef hwSetup_HPP
#define hwSetup_HPP
#include <Preferences.h>
#include <ETH.h>
#include <DNSServer.h>


  void hwSetupNodeMcuCytronNmea();
  void hwSetupF9PIoBoardNmea();
  void hwSetupInitial();
  void hwSetupWebSetup();
  char* hwSetupHardwareIdToName(uint8_t setup);

#endif
