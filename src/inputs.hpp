#ifndef inputs_HPP
#define inputs_HPP
#include <stdint.h>


constexpr int inputsHysteresis = 2;

struct InputsWasData {
  bool invertSensor;
  float center;
  float degreMultiplier;
  float statusRaw;
  float statusDegrees;
};
extern InputsWasData inputsWasSetup;
extern int inputsWasWebStatus;

void inputsSwitchesInit();
void inputsSwitchesTask(void *z);

void inputsWheelAngleInit();
void inputsWheelAngleTask(void *z);
void inputsWheelAngleStatusUpdate();

#endif
