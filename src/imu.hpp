#ifndef imu_HPP
#define imu_HPP
#include <stdint.h>


// updates the data, accel, gyros, magnetometer (each x, y, z)
extern void (*imuReadData)(float*, float*, float*);

struct ImuSettings {
  int8_t axis = 0;
  float rollOffSet = 0;
  bool invertRoll = false;
  float roll = 9999/16.0;
  bool sendRoll = false;
};
extern ImuSettings imuSettings;

void imuInit();
void imuTask(void *z);
void imuStatusUpdate();

#endif
