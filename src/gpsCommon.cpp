#include "gpsCommon.hpp"
#include "main.hpp"
#include <Preferences.h>
#include "webUi.hpp"
#include <ESPUI.h>
#include <stdio.h>
#include <HTTPClient.h>

GpsCommonRtcm gpsCommonRtcmSettings;
GpsCommonNmeaOutput gpsCommonNmeaOutput;
AsyncUDP gpsCommonUdpSocket;
constexpr uint gpsCommonPortDataToAog = 9999;
constexpr uint gpsCommonPortOwn = 5588;



void gpsCommonSetupRtcm(GpsCommonRtcm::RtcmDestination rtcmdestination) {
  gpsCommonRtcm.rtcmSource = (GpsCommonRtcm::RtcmSource)preferences.getUChar("gpsCommonRtcmSource",0);
  gpsCommonRtcm.rtcmDestination = rtcmdestination;

  // web
  uint16_t sel = ESPUI.addControl( ControlType::Select, "RTCM source", String( (int)gpsCommonRtcmSettings.rtcmSource ) , ControlColor::Wetasphalt, webTabGPS,
    []( Control * control, int id ) {
      preferences.putUChar("gpsCommonRtcmSource", control->value.toInt());
      webChangeNeedsReboot();
    } );
  ESPUI.addControl( ControlType::Option, "None", "0", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "UDP", "1", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "BlueTooth", "2", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "Ntrip", "3", ControlColor::Alizarin, sel );
  // only ntrip needs more options
  if (gpsCommonRtcm.rtcmSource == GpsCommonRtcm::RtcmSource::Ntrip) {
    ESPUI.addControl( ControlType::Text, "Ntrip Server", preferences.getString("gpsCommonNtripServer", ""), ControlColor::Wetasphalt, webTabGPS,
      []( Control * control, int id ) {
        preferences.putString("gpsCommonNtripServer", control->value);
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
    int control = ESPUI.addControl( ControlType::Number, "Ntrip port", String(preferences.getUInt("gpsCommonNtripPort", 2101)), ControlColor::Wetasphalt, webTabGPS,
      []( Control * control, int id ) {
        preferences.putUInt("gpsCommonNtripPort", control->value.toInt());
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
    ESPUI.addControl( ControlType::Min, "Min", "1", ControlColor::Peterriver, control );
    ESPUI.addControl( ControlType::Max, "Max", "65535", ControlColor::Peterriver, control );
    ESPUI.addControl( ControlType::Step, "Step", "1", ControlColor::Peterriver, control );
    ESPUI.addControl( ControlType::Text, "Ntrip Mountpoint", preferences.getString("gpsCommonNtripMount", ""), ControlColor::Wetasphalt, webTabGPS,
      []( Control * control, int id ) {
        preferences.putString("gpsCommonNtripMount", control->value);
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
    ESPUI.addControl( ControlType::Text, "Ntrip user", preferences.getString("gpsCommonNtripUser", ""), ControlColor::Wetasphalt, webTabGPS,
      []( Control * control, int id ) {
        preferences.putString("gpsCommonNtripUser", control->value);
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
    ESPUI.addControl( ControlType::Text, "Ntrip password", preferences.getString("gpsCommonNtripPassword", ""), ControlColor::Wetasphalt, webTabGPS,
      []( Control * control, int id ) {
        preferences.putString("gpsCommonNtripPassword", control->value);
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
    ESPUI.addControl( ControlType::Number, "Ntrip GGA interval", String(preferences.getUInt("gpsCommonNtripGga", 30)), ControlColor::Wetasphalt, webTabGPS,
      []( Control * control, int id ) {
        preferences.putUInt("gpsCommonNtripPort", control->value.toInt());
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
  }

  // init source
  switch (gpsCommonRtcm.rtcmSource) {
    case GpsCommonRtcm::RtcmSource::none:
      // no source, nothing to do
      break;
    case GpsCommonRtcm::RtcmSource::UDP:
      if ( !gpsCommonUdpSocket.connected() && !gpsCommonUdpSocket.listen(gpsCommonPortOwn)) {
        usb.println ("ERROR: Starting UDP Listener for rtcm failed");
      }
      gpsCommonCreateRtcmReceiveHandler();
      break;
    case GpsCommonRtcm::RtcmSource::BlueTooth: {
        String btName = preferences.getString("networkHostname", "ESP-AOG");
        btName += " GPS";
        if ( !gpsCommonBtSerial.isReady(false) && !gpsCommonBtSerial.begin(btName)) {
          usb.println ("ERROR: Starting Bluetooth for rtcm failed");
        }
        xTaskCreate( gpsCommonBtRtcmReceiver, "BT RTCM Receiver", 4096, NULL, 4, NULL );
      }
      break;
    case GpsCommonRtcm::RtcmSource::Ntrip:
      xTaskCreate( gpsCommonNtripReceiver, "Ntrip Receiver", 4096, NULL, 4, NULL );
      break;
}

}

void gpsCommonCreateRtcmReceiveHandler() {
  gpsCommonUdpSocket.onPacket([](AsyncUDPPacket packet) {
      gpsCommonRtcm.rtcmStatus = GpsCommonRtcm::RtcmStatus::working;
      while (packet.peek() != -1) {
        byte data = (byte)packet.read();
        if ( gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::gps1 || gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::both ) {
          gps1.write(data);
        }
        if ( gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::gps2 || gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::both ) {
          gps2.write(data);
        }
      }
    }
  );
}

void gpsCommonBtRtcmReceiver( void* z ) {
  while(1) {
    while (gpsCommonBtSerial.available()) {
      gpsCommonRtcm.rtcmStatus = GpsCommonRtcm::RtcmStatus::working;
      byte data = (byte)gpsCommonBtSerial.read();
      if ( gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::gps1 || gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::both ) {
        gps1.write(data);
      }
      if ( gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::gps2 || gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::both ) {
        gps2.write(data);
      }
    }
    vTaskDelay( 3 / portTICK_PERIOD_MS );
  }
}

void gpsCommonNtripReceiver( void* z ) {

  vTaskDelay( 1000 );


  String rtkCorrectionURL;
  rtkCorrectionURL.reserve( 200 );
  rtkCorrectionURL = "http://";

  if ( preferences.getString("gpsCommonNtripUser", "").length() > 1 ) {
    rtkCorrectionURL += preferences.getString("gpsCommonNtripUser", "");
    rtkCorrectionURL += ":";
    rtkCorrectionURL += preferences.getString("gpsCommonNtripPassword", "");
    rtkCorrectionURL += "@";
  }

  rtkCorrectionURL += preferences.getString("gpsCommonNtripServer", "");

  rtkCorrectionURL += ":";
  rtkCorrectionURL += preferences.getUInt("gpsCommonNtripPort", 2101);

  rtkCorrectionURL += "/";
  rtkCorrectionURL += preferences.getString("gpsCommonNtripMount", "");

  if ( rtkCorrectionURL.length() <= 8 ) {
    usb.println("Abort ntrip client, too short url");
    gpsCommonRtcm.rtcmStatus = GpsCommonRtcm::RtcmStatus::error;
    // delete this task
    TaskHandle_t myself = xTaskGetCurrentTaskHandle();
    vTaskDelete( myself );

    return;
  }

  // loop
  for ( ;; ) {
    HTTPClient http;
    http.begin( rtkCorrectionURL );
    http.setUserAgent( "NTRIP CoffeetracNTRIPClient" );
    int httpCode = http.GET();
    uint ggaInterval = preferences.getUInt("gpsCommonNtripGga", 30) * 1000;

    if ( httpCode > 0 ) {
      // HTTP header has been send and Server response header has been handled

      // file found at server
      if ( httpCode == HTTP_CODE_OK ) {
        gpsCommonRtcm.rtcmStatus = GpsCommonRtcm::RtcmStatus::working;

        // create buffer for read
        constexpr uint16_t buffSize = 1436;
        uint8_t* buff = ( uint8_t* )malloc( buffSize );

        // get tcp stream
        WiFiClient* stream = http.getStreamPtr();

        time_t timeoutSendGGA = millis() + ggaInterval;

        // read all data from server
        while ( http.connected() ) {
          // get available data size
          size_t size = stream->available();

          if ( size ) {
            int c = stream->readBytes( buff, ( ( size > buffSize ) ? buffSize : size ) );

            // write it to gps
            if ( gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::gps1 || gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::both ) {
              gps1.write( buff, c );
            }
            if ( gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::gps2 || gpsCommonRtcm.rtcmDestination == GpsCommonRtcm::RtcmDestination::both ) {
              gps2.write( buff, c );
            }
          }

          // send own position to ntrip server
          if ( millis() > timeoutSendGGA ) {
            if ( gpsCommonNmeaOutput.lastGGA.length() > 10 ) {
              if ( gpsCommonNmeaOutput.lastGGA.lastIndexOf( '\n' ) == -1 ) {
                gpsCommonNmeaOutput.lastGGA += "\r\n";
              }

              stream->write( gpsCommonNmeaOutput.lastGGA.c_str(), gpsCommonNmeaOutput.lastGGA.length() );
            }

            timeoutSendGGA = millis() + ggaInterval;
          }

          vTaskDelay( 3 );
        }
      }
    }

    // update WebUI
    gpsCommonRtcm.rtcmStatus = GpsCommonRtcm::RtcmStatus::error;

    http.end();
    vTaskDelay( 1000 );
  }

// delete this task
  TaskHandle_t myself = xTaskGetCurrentTaskHandle();
  vTaskDelete( myself );
}
