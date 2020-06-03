#include <ESPUI.h>

#include "main.hpp"
#include "gpsCommon.hpp"
#include "webUi.hpp"

AsyncUDP gpsCommonUdpSocket;

GpsRtcmData gpsRtcmData;
GpsNmeaOutput gpsNmeaOutput;

void gpsSendNmeaString(String data) {
  if (gpsNmeaOutput.udpOutput) {
    gpsCommonUdpSocket.broadcastTo( ( uint8_t* )data.c_str(), ( uint16_t )data.length(), 9999 );
  }
  if (gpsNmeaOutput.serialOutput) {
    rs232.print(data);
  }
  gpsNmeaOutput.lastSent = millis();
}

void gpsCommonInit() {
  // nnmea outputs
  gpsNmeaOutput.udpOutput = preferences.getBool("gpsOutUdp");
  ESPUI.addControl( ControlType::Switcher, "UDP Output", String( (int)gpsNmeaOutput.udpOutput ) , ControlColor::Wetasphalt, webTabGPS,
    []( Control * control, int id ) {
      gpsNmeaOutput.udpOutput = (boolean)control->value.toInt();
      preferences.putBool("gpsOutUdp", gpsNmeaOutput.udpOutput);
      control->color = ControlColor::Carrot;
    } );
  gpsNmeaOutput.serialOutput = preferences.getBool("gpsOutSerial");
  ESPUI.addControl( ControlType::Switcher, "Serial Output", String( (int)gpsNmeaOutput.serialOutput ) , ControlColor::Wetasphalt, webTabGPS,
    []( Control * control, int id ) {
      gpsNmeaOutput.serialOutput = (boolean)control->value.toInt();
      preferences.putBool("gpsOutSerial", gpsNmeaOutput.serialOutput);
      control->color = ControlColor::Carrot;
    } );
  // init if neccessary
  if ( gpsNmeaOutput.udpOutput ) {
    if ( !gpsCommonUdpSocket.connected() && !gpsCommonUdpSocket.listen(gpsCommonPortOwn)) {
      usb.println ("ERROR: Starting UDP Listener for sending nmea over udp failed");
    }
  }
}
