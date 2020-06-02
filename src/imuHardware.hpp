#ifndef imuHardware_HPP
#define imuHardware_HPP
#include <Preferences.h>
#include <ETH.h>
#include <DNSServer.h>
#include <SparkFunLSM9DS1.h>


extern LSM9DS1 imuHardwareLsm9Ds1;

bool imuHardwareLSM9DS1Init(uint8_t magAddr, uint8_t accAddr);
void imuHardwareLSM9DS1Aquire(float* ax, float* ay, float* az);

#endif
