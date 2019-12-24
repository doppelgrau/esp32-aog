#include <ESPUI.h>

#include "main.hpp"
#include "gpsCommon.hpp"
#include "webUi.hpp"

BluetoothSerial gpsCommonBtSerial;
int gpsCommonWebStatus;

AsyncUDP gpsCommonUdpSocket;

GpsRtcmData gpsRtcmData;
GpsNmeaOutput gpsNmeaOutput;

void gpsSendNmeaString(String data) {

  gpsNmeaOutput.lastSent = millis();
}

void gpsCommonInit() {
  // Status
  gpsCommonWebStatus = ESPUI.addControl( ControlType::Label, "Status:", "", ControlColor::Turquoise, webTabGPS );

  // nnmea outputs
  gpsNmeaOutput.udpOutput = preferences.getBool("gpsNmeaOutUdp");
  ESPUI.addControl( ControlType::Switcher, "UDP Output", String( (int)gpsNmeaOutput.udpOutput ) , ControlColor::Wetasphalt, webTabGPS,
    []( Control * control, int id ) {
      gpsNmeaOutput.udpOutput = (boolean)control->value.toInt();
      preferences.putBool("gpsNmeaOutUdp", gpsNmeaOutput.udpOutput);
    } );
  gpsNmeaOutput.serialOutput = preferences.getBool("gpsNmeaOutSerial");
  ESPUI.addControl( ControlType::Switcher, "Serial Output", String( (int)gpsNmeaOutput.serialOutput ) , ControlColor::Wetasphalt, webTabGPS,
    []( Control * control, int id ) {
      gpsNmeaOutput.serialOutput = (boolean)control->value.toInt();
      preferences.putBool("gpsNmeaOutSerial", gpsNmeaOutput.serialOutput);
    } );
  gpsNmeaOutput.btOutput = preferences.getBool("gpsNmeaOutBluetooth");
  ESPUI.addControl( ControlType::Switcher, "Bluetooth Output", String( (int)gpsNmeaOutput.btOutput ) , ControlColor::Wetasphalt, webTabGPS,
    []( Control * control, int id ) {
      gpsNmeaOutput.btOutput = (boolean)control->value.toInt();
      preferences.putBool("gpsNmeaOutBluetooth", gpsNmeaOutput.btOutput);
    } );
  // init if neccessary
  if ( gpsNmeaOutput.udpOutput ) {
    if ( !gpsCommonUdpSocket.connected() && !gpsCommonUdpSocket.listen(gpsCommonPortOwn)) {
      usb.println ("ERROR: Starting UDP Listener for sending nmea over udp failed");
    }
  }
  if ( gpsNmeaOutput.btOutput ) {
    String btName = preferences.getString("networkHostname", "ESP-AOG");
    btName += " GPS";
    if ( !gpsCommonBtSerial.isReady(false) && !gpsCommonBtSerial.begin(btName)) {
      usb.println ("ERROR: Starting Bluetooth for nmea failed");
    }
  }
}

void startGpsCommonStatus() {
  xTaskCreatePinnedToCore( gpsCommonStatusTask, "GPS-StatusWebUi", 4096, NULL, 2, NULL, 0 );
}

void gpsCommonStatusTask( void* z ) {
  constexpr TickType_t xFrequency = 1000;

  String str;
  str.reserve( 70 );

  while ( 1 ) {
    str = "Last sent: ";
    if (gpsNmeaOutput.lastSent == 0) {
      str += "never<br />";
    } else {
      str += millis() - gpsNmeaOutput.lastSent;
      str += "ms<br />";
    }
    str += "Quality: ";
    switch ( gpsNmeaOutput.qpsquality ) {
      case GpsNmeaOutput::GpsQuality::none:
        str += "No GPS Fix";
        break;
      case GpsNmeaOutput::GpsQuality::gps:
        str += "GPS Fix";
        break;
      case GpsNmeaOutput::GpsQuality::dgps:
        str += "DGPS Fix";
        break;
      case GpsNmeaOutput::GpsQuality::fixedrtk:
        str += "RTK Fix";
        break;
      case GpsNmeaOutput::GpsQuality::floatrtk:
        str += "RTK Float";
        break;
      default:
        str += "?";
        break;
    }
    str += "<br />RTCM: ";
    switch ( gpsRtcmData.rtcmStatus ) {
      case GpsRtcmData::RtcmStatus::setup:
        str += "Setup";
        break;
      case GpsRtcmData::RtcmStatus::working:
        str += "Working";
        break;
      case GpsRtcmData::RtcmStatus::error:
        str += "Error";
        break;
      default:
        str += "?";
        break;
    }

    Control* labelGpsStatus = ESPUI.getControl( gpsCommonWebStatus );
    labelGpsStatus->value = str;
    ESPUI.updateControl( labelGpsStatus );

    vTaskDelay( xFrequency );
  }
}
