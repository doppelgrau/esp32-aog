#include <ESPUI.h>
#include "main.hpp"
#include "imu.hpp"
#include "udpHandler.hpp"
#include "webUi.hpp"
#include "imuHardware.hpp"

void (*imuReadData)(float*, float*, float*);
int imuWebStatus;
ImuSettings imuSettings;

void imuInit() {
  imuWebStatus = ESPUI.addControl( ControlType::Label, "Status:", "No imu active", ControlColor::Turquoise, webTabIMU );

  // init the requested Imu
  uint8_t imuSelection = preferences.getUChar("imuChip", 0);
  uint16_t sel = ESPUI.addControl( ControlType::Select, "IMU", (String)imuSelection, ControlColor::Wetasphalt, webTabIMU,
    []( Control * control, int id ) {
      preferences.putUChar("imuChip", control->value.toInt());
      control->color = ControlColor::Carrot;
      ESPUI.updateControl( control );
      webChangeNeedsReboot();
    } );
  ESPUI.addControl( ControlType::Option, "None", "0", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "LSM9DS1 0x1C/0x6A", "1", ControlColor::Alizarin, sel );
  ESPUI.addControl( ControlType::Option, "LSM9DS1 0x1E/0x6B", "2", ControlColor::Alizarin, sel );

  // activate Imu
  switch (imuSelection) {
    case 1:
      if (!imuHardwareLSM9DS1Init(0x1C, 0x6A)) {
        status.hardwareStatus = Status::Hardware::error;
      }
      break;
      case 2:
        if (!imuHardwareLSM9DS1Init(0x1E, 0x6B)) {
          status.hardwareStatus = Status::Hardware::error;
        }
        break;
    default:
      break;
  }

  // imu
  //if (!imuHardwareLSM9DS1Init()) {
  //  hwInitErrors = false;
  //}

  if (imuReadData) {  // some IMU is configured
    // axis
    imuSettings.axis = preferences.getUChar("imuAxis", imuSettings.axis);
    sel = ESPUI.addControl( ControlType::Select, "Axis", (String)imuSettings.axis, ControlColor::Wetasphalt, webTabIMU,
      []( Control * control, int id ) {
        preferences.putUChar("imuAxis", control->value.toInt());
        imuSettings.axis = control->value.toInt();
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
      } );
    ESPUI.addControl( ControlType::Option, "X", "0", ControlColor::Alizarin, sel );
    ESPUI.addControl( ControlType::Option, "Y", "1", ControlColor::Alizarin, sel );
    ESPUI.addControl( ControlType::Option, "Z", "2", ControlColor::Alizarin, sel );

    imuSettings.rollOffSet = preferences.getFloat("imuRoll", imuSettings.rollOffSet);
    ESPUI.addControl( ControlType::Number, "Mounting correction Roll", (String)imuSettings.rollOffSet, ControlColor::Wetasphalt, webTabIMU,
      []( Control * control, int id ) {
        imuSettings.rollOffSet = control->value.toFloat();
        preferences.putFloat("imuRoll", imuSettings.rollOffSet);
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
      } );
    imuSettings.invertRoll = preferences.getBool("imuInvRoll");
    ESPUI.addControl( ControlType::Switcher, "Invert IMU roll", String( (int)imuSettings.invertRoll ) , ControlColor::Wetasphalt, webTabIMU,
      []( Control * control, int id ) {
        imuSettings.invertRoll = (boolean)control->value.toInt();
        preferences.putBool("imuInvRoll", imuSettings.invertRoll );
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
      } );
    imuSettings.sendRoll = preferences.getBool("imuSendRoll");
    ESPUI.addControl( ControlType::Switcher, "Send IMU roll", String( (int)imuSettings.sendRoll ) , ControlColor::Wetasphalt, webTabIMU,
      []( Control * control, int id ) {
        imuSettings.sendRoll = (boolean)control->value.toInt();
        preferences.putBool("imuSendRoll", imuSettings.sendRoll );
        control->color = ControlColor::Carrot;
        ESPUI.updateControl( control );
      } );

    // WebUI is ready, calibration data is loaded start task
    xTaskCreate( imuTask, "IMU", 8192, NULL, 4, NULL );
  }
}

void imuTask(void *z) {
  vTaskDelay( 2074 );
  constexpr TickType_t xFrequency = 20; // run at 50hz
  TickType_t xLastWakeTime = xTaskGetTickCount();
  // variables for the raw values
  float ax, ay, az;
  // for imu calibration

  // filter for roll and heading
  //Low pass butterworth filter order=2 alpha1=0.004
  //http://www.schwietering.com/jayduino/filtuino/index.php?characteristic=bu&passmode=lp&order=2&usesr=usesr&sr=50&frequencyLow=1&noteLow=&noteHigh=&pw=pw&calctype=float&run=Send
  class  FilterBuLp2_imu {
    public:
    FilterBuLp2_imu()
    {
      v[0]=0.0;
      v[1]=0.0;
    }
  private:
    float v[3];
  public:
  		float step(float x) //class II
  		{
  			v[0] = v[1];
  			v[1] = v[2];
  			v[2] = (1.551484234757205538e-4 * x)
  				 + (-0.96508117389913528061 * v[0])
  				 + (1.96446058020523239840 * v[1]);
  			return
  				 (v[0] + v[2])
  				+2 * v[1];
  		}
  } rollFilter;

  // debug
  const bool debugImu = false;
  int debugCounter = 0;

  // loop
  while (true) {
    // get new data
    imuReadData(&ax, &ay, &az);

    // calculate roll
    double roll = 0;
    switch(imuSettings.axis) {
      case 0:
        roll = ax;
        break;
      case 1:
        roll = ay;
        break;
      case 2:
        roll = az;
        break;
    }
    if (debugImu && (debugCounter % 50) == 0) {
      usb.print("Raw Roll: ");
      usb.println(roll);
    }
    if (imuSettings.invertRoll) {
      roll = roll * -1;
    }
    roll *= 90/9.8; //180/pi
    imuSettings.roll = rollFilter.step((float)roll);
    if (debugImu && (debugCounter++ % 50) == 0) {
      usb.print("Roll: ");
      usb.println(imuSettings.roll);
    }
    if (imuSettings.sendRoll) {
      udpActualData.roll = imuSettings.roll;
    }


    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }


}
