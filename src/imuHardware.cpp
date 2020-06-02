#include <imu.hpp>
#include <imuHardware.hpp>
#include <main.hpp>
#include <SparkFunLSM9DS1.h>

LSM9DS1 imuHardwareLsm9Ds1;

bool imuHardwareLSM9DS1Init(uint8_t magAddr, uint8_t accAddr) {
  // reset to make sure data is valid after "warmstart"
  if ( xSemaphoreTake( i2cMutex, 1000 ) == pdTRUE ) {
    Wire.beginTransmission(accAddr); // accelerometer/gyroscope
  	Wire.write(0x22);
  	Wire.write(3);
  	Wire.endTransmission();
    xSemaphoreGive( i2cMutex );
  }
  delay(10);
  uint lsm9ds1Init = 0;
  imuHardwareLsm9Ds1.settings.device.mAddress = magAddr; // Use I2C addres 0x1E
  imuHardwareLsm9Ds1.settings.device.agAddress = accAddr; // I2C address 0x6B
  if ( xSemaphoreTake( i2cMutex, 1000 ) == pdTRUE ) {
    lsm9ds1Init = imuHardwareLsm9Ds1.begin();
    usb.print("DEBUG: return code von LSM9DS1 begin(): ");
    usb.println(lsm9ds1Init);
    xSemaphoreGive( i2cMutex );
    if (lsm9ds1Init) {
      imuReadData = &imuHardwareLSM9DS1Aquire;
      usb.println("IMU Init - LSM9DS1 successfull");
    } else {
      usb.println("IMU Init - LSM9DS1 failed");
    }
  }

  return lsm9ds1Init != 0;
}

void imuHardwareLSM9DS1Aquire(float* ax, float* ay, float* az) {
  if ( xSemaphoreTake( i2cMutex, 1000 ) == pdTRUE ) {
    imuHardwareLsm9Ds1.readAccel();
    xSemaphoreGive( i2cMutex );
  }
  *ax = imuHardwareLsm9Ds1.calcAccel(imuHardwareLsm9Ds1.ax) * 9.8;
  *ay = imuHardwareLsm9Ds1.calcAccel(imuHardwareLsm9Ds1.ay) * 9.8;
  *az = imuHardwareLsm9Ds1.calcAccel(imuHardwareLsm9Ds1.az) * 9.8;
}
