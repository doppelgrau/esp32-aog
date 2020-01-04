#include "inputs.hpp"
#include "webUi.hpp"
#include "main.hpp"
#include "ioAccess.hpp"
#include "udpHandler.hpp"
#include <ESPUI.h>


// reads work & steerswitches
void inputsSwitchesInit() {
  // Webinterface
  // Workswitch
  uint16_t sel = ESPUI.addControl( ControlType::Select, "Workswitch input", (String)preferences.getUChar("inputsWsIo", 253), ControlColor::Wetasphalt, webTabWorkSteerSwitch,
    []( Control * control, int id ) {
      preferences.putUChar("inputsWsIo", control->value.toInt());
      control->color = ControlColor::Carrot;
      ESPUI.updateControl( control );
      webChangeNeedsReboot();
    } );
  ESPUI.addControl( ControlType::Option, "True", "254", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "False", "253", ControlColor::Alizarin, sel );
  ioAccessWebListAnalogIn(sel);

  ESPUI.addControl( ControlType::Number, "Workswitch switch-point in %", String(preferences.getUInt("inputsWsSp", 50)), ControlColor::Wetasphalt, webTabWorkSteerSwitch,
    []( Control * control, int id ) {
      preferences.putUInt("inputsWsSp", control->value.toInt());
      control->color = ControlColor::Carrot;
      ESPUI.updateControl( control );
      webChangeNeedsReboot();
    } );

    // Steer switch
    sel = ESPUI.addControl( ControlType::Select, "Steerswitch input", (String)preferences.getUChar("inputsSsIo", 253), ControlColor::Wetasphalt, webTabWorkSteerSwitch,
      []( Control * control, int id ) {
        preferences.putUChar("inputsSsIo", control->value.toInt());
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
    ESPUI.addControl( ControlType::Option, "True", "254", ControlColor::Alizarin, sel );
    ESPUI.addControl( ControlType::Option, "False", "253", ControlColor::Alizarin, sel );
    ioAccessWebListAnalogIn(sel);

    ESPUI.addControl( ControlType::Number, "Steerswitch switch-point in %", String(preferences.getUInt("inputsSsSp", 50)), ControlColor::Wetasphalt, webTabWorkSteerSwitch,
      []( Control * control, int id ) {
        preferences.putUInt("inputsWsSp", control->value.toInt());
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );

    ESPUI.addControl( ControlType::Switcher, "Steerswitch invert value", String( (int)preferences.getBool("inputsSsInv") ) , ControlColor::Wetasphalt, webTabWorkSteerSwitch,
      []( Control * control, int id ) {
        preferences.putBool("inputsSsInv", (boolean)control->value.toInt());
        control->color = ControlColor::Carrot;
        webChangeNeedsReboot();
      } );

    ESPUI.addControl( ControlType::Switcher, "Steerswitch is button", String( (int)preferences.getBool("inputsSsButton") ) , ControlColor::Wetasphalt, webTabWorkSteerSwitch,
      []( Control * control, int id ) {
        preferences.putBool("inputsSsButton", (boolean)control->value.toInt());
        control->color = ControlColor::Carrot;
        webChangeNeedsReboot();
      } );

    // start task
    xTaskCreate( inputsSwitchesTask, "Switches", 4096, NULL, 4, NULL );

}

// against gliches/noise: two values must show same value for a change + a hysteresis
void inputsSwitchesTask(void *z) {
  bool steerSwitchLastValidState = false;
  bool steerSwitchLastState = false;
  bool workSwitchLastState = false;
  bool steerSwitchIsButton = preferences.getBool("inputsSsButton");
  bool steerSwitchInvert  = preferences.getBool("inputsSsInv");
  uint8_t steerSwitchThreshold = preferences.getUInt("inputsSsSp", 50);
  uint8_t workSwitchThreshold = preferences.getUInt("inputsWsSp", 50);
  uint8_t steerSwitchPort = preferences.getUInt("inputsSsIo", 253);
  uint8_t workSwitchPort = preferences.getUInt("inputsWsIo", 253);


  for ( ;; ) {
    // workSwitch
    float currentValue = fabs(ioAccessGetAnalogInput(workSwitchPort));
    if (udpActualData.workSwitch) {
      if (currentValue < ((workSwitchThreshold - inputsHysteresis)/100.0)) {
        if (workSwitchLastState == false) {
          udpActualData.workSwitch = false;
        }
        workSwitchLastState = false;
      } else {
        workSwitchLastState = true;
      }
    } else { // current workswitch false
      if (currentValue > ((workSwitchThreshold + inputsHysteresis)/100.0)) {
        if (workSwitchLastState == true) {
          udpActualData.workSwitch = true;
        }
        workSwitchLastState = true;
      } else {
        workSwitchLastState = false;
      }
    }

    // steerswitch
    currentValue = fabs(ioAccessGetAnalogInput(steerSwitchPort));
    if (steerSwitchInvert) { // invert the raw value => hysteresislogic has only one case, also the "rising edge" logik for the button
      currentValue = 1.0 - currentValue;
    }
    bool newValue = steerSwitchLastValidState;
    if (steerSwitchLastValidState) {
      if (currentValue < ((steerSwitchThreshold - inputsHysteresis)/100.0)) {
        newValue = false;
      } else {
        newValue = true;
      }
    } else { // current steerswitch false
      if (currentValue > ((steerSwitchThreshold + inputsHysteresis)/100.0)) {
        newValue = true;
      } else {
        newValue = false;
      }
    }
    if (!steerSwitchLastValidState && steerSwitchLastState && newValue) { // false => true
      steerSwitchLastValidState = true;
      if (steerSwitchIsButton) {
        udpActualData.steerSwitch = !udpActualData.steerSwitch;
      } else {
        udpActualData.steerSwitch = true;
      }
    } else if (steerSwitchLastValidState && !steerSwitchLastState && !newValue) { // true => false
      steerSwitchLastValidState = false;
      if (!steerSwitchIsButton) {
        udpActualData.steerSwitch = false;
      }
    }
    steerSwitchLastState = newValue;

    vTaskDelay( 16 );
  }
}

// calculates steering angle
