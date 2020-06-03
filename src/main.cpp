// MIT License
//
// Copyright (c) 2019 Christian Riggenbach
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <string.h>
#include <Preferences.h>
// #include <SoftwareSerial.h>
#include <DNSServer.h>
#include <ESPUI.h>

#include "main.hpp"
#include "hwSetup.hpp"
#include "webUi.hpp"
#include "idleStats.hpp"
#include "ioAccess.hpp"
#include "udpHandler.hpp"
#include "gpsCommon.hpp"
#include "idleStats.hpp"
#include "network.hpp"
#include "inputs.hpp"
#include "uturn.hpp"
#include "steering.hpp"
#include "imu.hpp"

///////////////////////////////////////////////////////////////////////////
// global data
///////////////////////////////////////////////////////////////////////////
SemaphoreHandle_t i2cMutex, preferencesMutex;
Status status;
Preferences preferences;

HardwareSerial usb(Serial);
HardwareSerial gps1(Serial1);
// SoftwareSerial gps2;
HardwareSerial rs232(Serial2);

void setup() {
  usb.begin(115200);
  usb.println("\nSetup");
  // Init I2C
  i2cMutex = xSemaphoreCreateMutex();

  // preferences
  usb.println("\tOpen preferences");
  preferences.begin("config", false);


  // prepare webinterface
  usb.println("\tCore webinterface");
  webInitCore();
  // chose hardware
  usb.println("\tHW-Setup Webinterface");
  hwSetupWebSetup();

  // start LED-Task early, so it can be used for the HW-setup
  xTaskCreate( statusLedWorker, "Status-LED", 2048, NULL, 1, NULL );

  // read configuration for setup
  usb.print("\tGet HW setup number: ");

  uint8_t hwSetup = preferences.getUChar("hwSetup");
  usb.println(hwSetup);

  usb.println("\nbeginn HW-Init");

  switch (hwSetup) {
    case 1:
      hwSetupNodeMcuCytronNmea();
      break;
    case 2:
      hwSetupF9PIoBoardNmea();
      break;
    case 3:
      hwSetupNodeMcuIbt2Nmea();
      break;
    default:
      hwSetupInitial();
      break;
  }

  // generic initializations
  if (hwSetup != 0) {
    inputsSwitchesInit();
    inputsWheelAngleInit();
    uturnInit();
    steeringInit();
    imuInit();
  }
  // set up webinterface
  webStart();

  // Set up some common thread
  initIdleStats();
  udpHandlerInit();
  //xTaskCreate( statusWebWorker, "Status-Web", 2048, NULL, 1, NULL );
}

void loop( void ) {
  if (status.networkStatus == Status::Network::accessPoint && WiFi.status() == WL_CONNECTED) {
    hwSetupDnsServer.processNextRequest();
  }
  vTaskDelay( 5 );
}

void statusWebWorker( void* z ) {
  TickType_t lastStatusUpdate = 0;

  while ( 1 ) {
    TickType_t  now = xTaskGetTickCount();
    if ( now - lastStatusUpdate >= 999) {
      lastStatusUpdate = now;
      idleStats();
      delay(350);
      statusCommunication();
      delay(350);
      statusInputs();
    }
    vTaskDelay( 2 );
  }
}

void statusCommunication() {
  String str;
  str.reserve( 120 );

  str = "PGN recv: ";
  str += udpHandlerTimeGenerator(udpAogData.lastReceived7FFE);
  str += "<br/>";
  str += "PGN send: ";
  str += udpHandlerTimeGenerator(udpActualData.lastSent);
  str += "<br />GPS sent: ";
  if (gpsNmeaOutput.lastSent == 0) {
    str += "never<br />";
  } else {
    str += millis() - gpsNmeaOutput.lastSent;
    str += "ms";
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
  Control* labelPgnStatus = ESPUI.getControl( webLabelCommStatus );
  labelPgnStatus->value = str + " ";
  ESPUI.updateControl( webLabelCommStatus );
}

void statusInputs() {
  String str;
  str.reserve( 120 );
  str = "Roll: ";
  if (imuReadData) {
    str += String(imuSettings.roll,1);
  } else {
    str += "NA";
  }
  str += "<br/>GPS Quality: ";
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
  str += "<br/>WAS Raw: ";
  str += String(inputsWasSetup.statusRaw, 3);
  str += "<br />WAS final: ";
  str += String(udpActualData.steerAngleActual, 1);


  Control* label = ESPUI.getControl( webLabelInputStatus );
  label->value = str + " ";
  ESPUI.updateControl( webLabelInputStatus );

}

void statusLedWorker( void* z ) {
  uint8_t position = 0;

  while ( 1 ) {
    // generate pattern from current status
    uint32_t statusLedPattern = 0b0000111000<<22;
    switch (status.networkStatus) {
      case Status::Network::connecting:
        statusLedPattern += 0b01111100000<<11;
        break;
      case Status::Network::connected:
        statusLedPattern += 0b01111111111<<11;
        break;
      case Status::Network::accessPoint:
        statusLedPattern += 0b01110000111<<11;
        break;
      default:
         statusLedPattern += 0b01100110011<<11;
        break;
    }
    switch (status.hardwareStatus) {
      case Status::Hardware::ok:
        statusLedPattern += 0b01111111111;
        break;
      default:
         statusLedPattern += 0b01100110011;
        break;
    }

    // shift a bit and get first bit
    bool ledState = ( statusLedPattern >> position ) & 1;
    ioAccessSetDigitalOutput(status.statusPort, ledState);

    // increase counter
    position = (position + 1) % 32;
    // Wait
    vTaskDelay( 62 / portTICK_PERIOD_MS );
  }
}
