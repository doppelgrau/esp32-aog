#include "inputs.hpp"
#include "webUi.hpp"
#include "main.hpp"
#include "ioAccess.hpp"
#include "udpHandler.hpp"
#include <ESPUI.h>

int inputsWasWebStatus;
InputsWasData inputsWasSetup;

// filter for steering angle
// http://www.schwietering.com/jayduino/filtuino/index.php?characteristic=bu&passmode=lp&order=2&usesr=usesr&sr=100&frequencyLow=5&noteLow=&noteHigh=&pw=pw&calctype=float&run=Send
//Low pass butterworth filter order=2 alpha1=0.05
class  FilterBuLp2_3 {
  public:
    FilterBuLp2_3() {
      v[0] = 0.0;
      v[1] = 0.0;
    }
  private:
  		float v[3];
  	public:
  		float step(float x) { //class II
  			v[0] = v[1];
  			v[1] = v[2];
  			v[2] = (6.745527388907189559e-2 * x)
  				 + (-0.41280159809618854894 * v[0])
  				 + (1.14298050253990091107 * v[1]);
  			return
  				 (v[0] + v[2])
  				+2 * v[1];
  		}
} wheelAngleSensorFilter;


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

// calculates wheel angle
void inputsWheelAngleInit() {
  inputsWasWebStatus = ESPUI.addControl( ControlType::Label, "Status:", "", ControlColor::Turquoise, webTabSteeringAngle );
  uint16_t sel = ESPUI.addControl( ControlType::Select, "Wheel angle sensor input", (String)preferences.getUChar("inputsWasIo", 255), ControlColor::Wetasphalt, webTabSteeringAngle,
    []( Control * control, int id ) {
      preferences.putUChar("inputsWasIo", control->value.toInt());
      control->color = ControlColor::Carrot;
      ESPUI.updateControl( control );
      webChangeNeedsReboot();
    } );
  ESPUI.addControl( ControlType::Option, "None", "255", ControlColor::Alizarin, sel );
  ioAccessWebListAnalogIn(sel);
  inputsWasSetup.invertSensor = preferences.getBool("inputsWasInv");
  ESPUI.addControl( ControlType::Switcher, "Invert Signal", String( (int)inputsWasSetup.invertSensor ) , ControlColor::Wetasphalt, webTabGPS,
    []( Control * control, int id ) {
      inputsWasSetup.invertSensor = (boolean)control->value.toInt();
      preferences.putBool("inputsWasInv", inputsWasSetup.invertSensor);
      control->color = ControlColor::Carrot;
    } );
  inputsWasSetup.center = preferences.getFloat("inputsWasCenter", 0.5);
  ESPUI.addControl( ControlType::Number, "Wheel angle sensor center", (String)inputsWasSetup.center, ControlColor::Wetasphalt, webTabSteeringAngle,
    []( Control * control, int id ) {
      inputsWasSetup.center = control->value.toFloat();
      preferences.putFloat("inputsWasCenter", inputsWasSetup.center);
      control->color = ControlColor::Carrot;
      ESPUI.updateControl( control );
    } );

  sel = ESPUI.addControl( ControlType::Select, "Correktion", (String)preferences.getUChar("inputsWasCorr", 0), ControlColor::Wetasphalt, webTabSteeringAngle,
    []( Control * control, int id ) {
      preferences.putUChar("inputsWasCorr", control->value.toInt());
      control->color = ControlColor::Carrot;
      ESPUI.updateControl( control );
      webChangeNeedsReboot();
    } );
  ESPUI.addControl( ControlType::Option, "None", "0", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "Ackermann (sensor left)", "1", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "Ackermann (sensor right)", "2", ControlColor::Alizarin, sel );
  sel = preferences.getUChar("inputsWasCorr", 0);
  if ( sel == 1 || sel == 2) {
    // data for Ackermann korrection
    ESPUI.addControl( ControlType::Number, "Wheelbase (cm)", (String)preferences.getInt("inputsWasWB", 450), ControlColor::Wetasphalt, webTabSteeringAngle,
      []( Control * control, int id ) {
        preferences.putInt("inputsWasWB", control->value.toInt());
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
    ESPUI.addControl( ControlType::Number, "Track width (cm)", (String)preferences.getInt("inputsWasTW", 200), ControlColor::Wetasphalt, webTabSteeringAngle,
      []( Control * control, int id ) {
        preferences.putInt("inputsWasTW", control->value.toInt());
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
        webChangeNeedsReboot();
      } );
  } // end of ackermann
  inputsWasSetup.degreMultiplier = preferences.getFloat("inputsWasMult", 75.5);
  ESPUI.addControl( ControlType::Number, "Multiplier to degrees", (String)inputsWasSetup.degreMultiplier, ControlColor::Wetasphalt, webTabSteeringAngle,
    []( Control * control, int id ) {
      inputsWasSetup.degreMultiplier = control->value.toFloat();
      preferences.putFloat("inputsWasMult", inputsWasSetup.degreMultiplier);
      control->color = ControlColor::Carrot;
      ESPUI.updateControl( control );
    } );

// start task
xTaskCreate( inputsWheelAngleTask, "WAS", 4096, NULL, 8, NULL );

} // end WAS init

void inputsWheelAngleTask(void *z) {
  constexpr TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  int wheelbase = preferences.getInt("inputsWasWB", 450);
  int trackWidth = preferences.getInt("inputsWasTW", 200);
  uint8_t correction = preferences.getUChar("inputsWasCorr", 0);
  uint8_t inputPort = preferences.getUChar("inputsWasIo", 255);
  while (1) {
    float newInput = fabs(ioAccessGetAnalogInput(inputPort)); // use fabs is the signal goes to negative numbers, eg. uses 5V for preferences
    inputsWasSetup.statusRaw = newInput;
    if (inputsWasSetup.invertSensor) {
      newInput = 1 - newInput;
    }
    // center = 0
    newInput = newInput - inputsWasSetup.center;

    // multiply to get the degres
    newInput = newInput * inputsWasSetup.degreMultiplier;
    inputsWasSetup.statusDegrees = newInput;

    // Ackermann (ignore everything below 0,5°)
    if ( (correction == 1  || correction == 2 ) && (newInput > 0.5 || newInput < 0.5)) {
      // just for the human, nicer names
      bool negativeAngle = newInput < 0;
      float mathAngle = abs(newInput) * PI / 180;

      // calculate the distance of the adjacent side of the triangle (turning point rear axle <-> turn circle center)
      float distance = wheelbase / tan( mathAngle );
      // add or substract half the trackWidth
      if ( ( negativeAngle && correction == 1 )
          || ( ! negativeAngle && correction == 2 ) ) {
          distance += trackWidth / 2;
      } else {
        distance -= trackWidth / 2;
      }

      // now calculate the virtual wheel in the center
      mathAngle = atan(wheelbase / distance);

      // convert back to degrees and add go back to negative/positive
      if (negativeAngle) {
        newInput = mathAngle * 180 / PI * -1;
      } else {
        newInput = mathAngle * 180 / PI;
      }
    } // end of Ackermann

    // filter the values a bit
    newInput = wheelAngleSensorFilter.step( newInput );
    // update data
    udpActualData.steerAngleActual = newInput;

    // wait for next cycle
    vTaskDelayUntil( &xLastWakeTime, xFrequency );

  } // end while loop
}

void inputsWheelAngleStatusUpdate() {
  String str;
  str.reserve( 70 );

  str = "Raw: ";
  str += String(inputsWasSetup.statusRaw, 3);
  str += "<br />Raw degrees: ";
  str += String(inputsWasSetup.statusDegrees, 1);
  str += "<br />Final: ";
  str += String(udpActualData.steerAngleActual, 1);
  if (inputsWasWebStatus != 0 ){
    Control* labelGpsStatus = ESPUI.getControl( inputsWasWebStatus );
    labelGpsStatus->value = str;
    ESPUI.updateControl( inputsWasWebStatus );
  }
}
