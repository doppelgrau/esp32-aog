#include <WiFi.h>
#include <Preferences.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "hwSetup.hpp"


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
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
  }
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}
