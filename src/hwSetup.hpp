#ifndef hwSetup_HPP
#define hwSetup_HPP
#include <Preferences.h>
#include <ETH.h>

  void hwSetupNodeMcuCytronNmea();
  void hwSetupF9PIoBoardNmea();
  void hwSetupInitial();
  void hwSetupNetworkAp(bool emergencyMode = true);
  void hwSetupNetworkClient();
  void hwSetupNetworkLan8720(uint8_t phy_addr, int power, int mdc, int mdio);
  void hwSetupWebSetup();
  void hwSetupWebNetwork();
  char* hwSetupHardwareIdToName(uint8_t setup);
  void hwSetupWifiMonitor( void* z );
  void hwSetupEthernetEvent(WiFiEvent_t event);

  extern bool hwSetupHasEthernet;
#endif
