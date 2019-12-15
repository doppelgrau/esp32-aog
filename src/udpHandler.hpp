// should handle the normal messages from AOG on the PGN broadcast port
// not special like NTRIP on other ports

#include "main.hpp"

struct UdpFromAogData {
  uint lastReceived7FFE;
  int16_t distanceFromGuidanceLine;
  float requiredSteerAngle;
  byte uTurnRelais;
};
extern UdpFromAogData udpAogData;

struct UdpActualData {
  uint lastSent;

  float steerAngleActual;

  bool workSwitch;
  bool steerSwitch;
  bool remoteSwitch;

  int16_t pwm;

  float roll = 9999/16.0;
  float heading = 9999/16.0;

};
extern UdpActualData udpActualData;

void udpHandlerInit();
void udpHandlerCreateReceiveHandler();
String udpHandlerTimeGenerator(uint last);
