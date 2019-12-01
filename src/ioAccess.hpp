#include <Arduino.h>

#ifndef ioAccess_HPP
#define ioAccess_HPP

  bool ioAccessInitAsDigitalOutput(uint8_t port);
  bool ioAccessInitAsDigitalInput(uint8_t port, bool usePullUpDown, bool pullDirectionUp);
  bool ioAccessInitAsAnalogInput(uint8_t port);
  bool ioAccessInitPwmChannel(uint8_t channel);
  bool ioAccessInitAttachToPwmChannel(uint8_t port, uint8_t channel);

  void ioAccessSetDigitalOutput(uint8_t port, bool value);
  bool ioAccessGetDigitalInput(uint8_t port);
  float ioAccessGetAnalogInput(uint8_t port); // scaled to -1 to 1 (or 0-1 if no negative value is possible)

  bool ioAccess_FXL6408_init(uint8_t address) ;
  bool ioAccess_FXL6408_configureAsDigitalOutput(uint8_t address, uint8_t port);
  void ioAccess_FXL6408_setDigitalOutput(byte i2cAddress, uint8_t port, bool state);
  bool ioAccess_FXL6408_configureAsDigitalInput(byte i2cAddress, uint8_t port, bool usePullUpDown, bool pullDirectionUp);
  bool ioAccess_FXL6408_getDigitalOutput(byte i2cAddress, uint8_t port);
  uint8_t ioAccess_FXL6408_setByteI2C(byte i2cAddress, byte i2cregister, byte value);
  uint8_t ioAccess_FXL6408_getByteI2C(byte i2cAddress, int i2cregister);
#endif
