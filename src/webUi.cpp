#include <ESPUI.h>
#include "webUi.hpp"

uint16_t webLabelLoad;
uint16_t webButtonReboot;

uint16_t webTabHardware;
uint16_t webTabGPS;
uint16_t webTabIMU;
uint16_t webTabSteeringAngle;
int16_t webTabSteeringActuator;
uint16_t webTabUturn;
uint16_t webTabWorkSteerSwitch;

void webInitCore() {
    webLabelLoad = ESPUI.addControl( ControlType::Label, "Load:", "", ControlColor::Turquoise );
    webButtonReboot = ESPUI.addControl( ControlType::Button, "If this turn red, you have to", "Reboot", ControlColor::Emerald, Control::noParent,
      []( Control * control, int id ) {
        if ( id == B_UP ) {
          ESP.restart();
        }
      } );
    webTabHardware = ESPUI.addControl( ControlType::Tab, "Hardware", "Hardware" );
    webTabGPS = ESPUI.addControl( ControlType::Tab, "GPS", "GPS" );
    webTabIMU = ESPUI.addControl( ControlType::Tab, "IMU", "IMU" );
    webTabSteeringAngle = ESPUI.addControl( ControlType::Tab, "Steering angle", "Steering angle" );
    webTabSteeringActuator = ESPUI.addControl( ControlType::Tab, "Steering actuator", "Steering actuator" );
    webTabUturn = ESPUI.addControl( ControlType::Tab, "U-Turn", "U-Turn" );
    webTabWorkSteerSwitch = ESPUI.addControl( ControlType::Tab, "Work-/Steer switch", "Work-/Steer switch" );
}

void webStart() {
  ESPUI.begin("AgOpenGPS ESP32 controler");
}
void webChangeNeedsReboot(){
  ESPUI.getControl( webButtonReboot )->color = ControlColor::Alizarin;
  ESPUI.updateControl( webButtonReboot );
};
