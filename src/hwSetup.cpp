#include <WiFi.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "hwSetup.hpp"
#include "webUi.hpp"
#include <ESPUI.h>


DNSServer dnsServer;

void hwSetupWifiApOnly() {
  Preferences preferences;
  preferences.begin("hwSetup", true);

  // WiFi
  hwSetupNetworkAp(preferences);

  // TODO webUI

  // close preferences
  preferences.end();
}

void hwSetupNodeMcuNmea() {

}

void hwSetupF9PIoBoardNmea() {

}

void hwSetupNetworkAp(Preferences preferences) {
  IPAddress localIp = IPAddress( 172, 23, 42, 1 );
  char hostname[33];
  strcpy(hostname, "ESP-AOG"); // default
  preferences.getString("networkHostname", hostname, 33);

  WiFi.setHostname( hostname );
  WiFi.mode( WIFI_AP );
  delay(10);
  WiFi.softAPConfig( localIp, localIp, IPAddress( 255, 255, 255, 0 ) );
  WiFi.softAP( hostname );
  dnsServer.start( 53, "*", localIp );
}

void hwSetupWebSetup() {
  Preferences preferences;
  preferences.begin("aog", true);
  uint8_t hw = preferences.getUChar("hwSetup");
  preferences.end();

  // HW-"Plattform"
  if ( hw  == 0 ) {
    uint16_t sel = ESPUI.addControl( ControlType::Select, "Hardware", "0", ControlColor::Wetasphalt, webTabHardware,
      []( Control * control, int id ) {
        Preferences preferences;
        preferences.begin("aog", false);
        preferences.putUChar("hwSetup", control->value.toInt());
        preferences.end();
        webChangeNeedsReboot();
      } );
    ESPUI.addControl( ControlType::Option, hwSetupIdToName(0), "0", ControlColor::Alizarin, sel );
    ESPUI.addControl( ControlType::Option, hwSetupIdToName(1), "1", ControlColor::Alizarin, sel );
    ESPUI.addControl( ControlType::Option, hwSetupIdToName(2), "2", ControlColor::Alizarin, sel );
  } else {
    uint16_t sel = ESPUI.addControl( ControlType::Select, "Hardware", String(hw), ControlColor::Wetasphalt, webTabHardware,
      []( Control * control, int id ) {} );
    ESPUI.addControl( ControlType::Option, hwSetupIdToName(hw), String(hw), ControlColor::Alizarin, sel );
  }
  // Reset
  ESPUI.addControl( ControlType::Button, "Reset to Default", "Reset", ControlColor::Wetasphalt, webTabHardware,
    []( Control * control, int id ) {
      if ( id == B_UP && control->value.equals("Reset")) {
        control->value = "Really?";
        control->color = ControlColor::Alizarin;
        ESPUI.updateControl( control );
      } else if ( id == B_UP && control->value.equals("Really?")) {
        Preferences preferences;
        preferences.begin("aog", false);
        preferences.clear();
        preferences.end();
        webChangeNeedsReboot();
      }
    } );
}

char* hwSetupIdToName(uint8_t setup) {
  switch (setup) {
    case 1:
      return (char*)"NodeMCU Nmea";
      break;
    case 2:
      return (char*)"F9P-IO-Board Nmea";
      break;
    default:
      return (char*)"Initial Setup";
      break;
  }
}
