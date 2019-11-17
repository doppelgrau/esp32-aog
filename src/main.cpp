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

#include "main.hpp"
#include "hwSetup.hpp"
#include "webUi.hpp"


///////////////////////////////////////////////////////////////////////////
// global data
///////////////////////////////////////////////////////////////////////////
SemaphoreHandle_t i2cMutex, preferencesMutex;



void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nSetup");
  // Init I2C
  i2cMutex = xSemaphoreCreateMutex();

  // prepare webinterface
  webInitCore();

  // read configuration for setup
  Preferences preferences;
  preferences.begin("core", true);

  // chose hardware
  uint8_t hwSetup = preferences.getUChar("hwSetup");
  // close preferences
  preferences.end();

  switch (hwSetup) {
    case 1:
      hwSetupNodeMcuNmea();
      break;
    case 2:
      hwSetupF9PIoBoardNmea();
      break;
    default:
      hwSetupWifiApOnly();
      break;
  }

  // set up webinterface
  webStart();

  // Set up some common threads

}


void loop( void ) {
  Serial.println("Loop");
  vTaskDelay( 100 );
}
