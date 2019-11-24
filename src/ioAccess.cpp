#include "ioAccess.hpp"
#include "main.hpp"

/*
##
## IO Ports
##
Port 255 has special meaning (not configured)
Port 0-40 reserved for the respective ports on an esp32
Port 41-44 ADS1115 Adress 0x48 Port 0-3
Port 45 ADS1115 Adress 0x48 Differential Port 0/1
Port 46 ADS1115 Adress 0x48 Differential Port 2/3
Port 47-64 reserved for other possible ADS adresses
Port 65-72 FXL6408 Address 0x43
Port 73-80 FXL6408 Address 0x44

##
## PWM Channel
##
Channel 0-15 ESP32
*/

bool ioAccessInitAsDigitalOutput(uint8_t port) {
  switch (port) {
    case 0:
    case 2:
    case 4 ... 5:
    case 12 ... 33: {
        // all "normal" ESP32 Outputs, excluded serial to usb even if in theory usable
        gpio_num_t espPort = static_cast<gpio_num_t>(port);
        gpio_pad_select_gpio(espPort);
        gpio_intr_disable(espPort);
        gpio_set_pull_mode(espPort, GPIO_FLOATING);
        gpio_set_direction(espPort, GPIO_MODE_OUTPUT);
      };
      return true;
      break;
    case 65 ... 80:
      return ioAccess_FXL6408_configureAsDigitalOutput((0x43 + (port - 65) / 8), ((port - 65) % 8));
      break;
    default:
      return false;
  }
}

void ioAccessSetDigitalOutput(uint8_t port, bool value) {
  switch (port) {
    case 0:
    case 2:
    case 4 ... 5:
    case 12 ... 33: {
        // all "normal" ESP32 Outputs, excluded serial to usb even if in theory usable
        if (value) {
          digitalWrite(static_cast<gpio_num_t>(port), HIGH);
        } else {
          digitalWrite(static_cast<gpio_num_t>(port), LOW);
        }
      };
      break;
    case 65 ... 80:
      return ioAccess_FXL6408_setDigitalOutput((0x43 + (port - 65) / 8), ((port - 65) % 8), value);
      break;
  }
}

bool ioAccessInitAsDigitalInput(uint8_t port, bool usePullUpDown, bool pullDirectionUp) {
   switch (port) {
    case 0:
    case 2:
    case 4 ... 5:
    case 12 ... 39: {
        // all "normal" ESP32 Inputs, excluded serial to usb even if in theory usable
        gpio_num_t espPort = static_cast<gpio_num_t>(port);
        gpio_pad_select_gpio(espPort);
        gpio_set_direction(espPort, GPIO_MODE_INPUT);
        gpio_intr_disable(espPort);
        if (usePullUpDown) {
          if (pullDirectionUp) {
            gpio_set_pull_mode(espPort, GPIO_PULLUP_ONLY);
          } else {
            gpio_set_pull_mode(espPort, GPIO_PULLDOWN_ONLY);
          }
        } else {
          gpio_set_pull_mode(espPort, GPIO_FLOATING);
        }
      };
      return true;
      break;
    case 65 ... 80:
      return ioAccess_FXL6408_configureAsDigitalInput((0x43 + (port - 65) / 8), ((port - 65) % 8), usePullUpDown, pullDirectionUp);
      break;
    default:
      return false;
  }
}

bool ioAccessInitAsAnalogInput(uint8_t port);
bool ioAccessInitAttachToPwmChannel(uint8_t port, uint8_t channel);

bool ioAccessGetDigitalInput(uint8_t port);
float ioAccessGetAnalogInput(uint8_t port);


//helper
uint8_t setBit(uint8_t byte, uint8_t position, bool value) {
  uint8_t pattern = 0b00000001 << position;
  if (value) {
    return byte | pattern;
  }  else {
    pattern = ~pattern;
    return byte & pattern;
  }
}


// FXL6408
bool ioAccess_FXL6408_init(uint8_t address) {
  int returnValues = 0;
  returnValues += ioAccess_FXL6408_setByteI2C(address, 0x07, 0b11111111); // Output High-Z (not driven)
  returnValues += ioAccess_FXL6408_setByteI2C(address, 0x03, 0b11111111); // Everything Output
  returnValues += ioAccess_FXL6408_setByteI2C(address, 0x05, 0b00000000); // (Disabled) Outputs to low)
  returnValues += ioAccess_FXL6408_setByteI2C(address, 0x0B, 0b00000000); // No Pullup/down
  returnValues += ioAccess_FXL6408_setByteI2C(address, 0x11, 0b11111111); // No interrupts
  return returnValues == 0;
}

bool ioAccess_FXL6408_configureAsDigitalOutput(uint8_t address, uint8_t port) {
    int returnValues = 0;
    // default low
    ioAccess_FXL6408_Output[address - 0x43] = setBit(ioAccess_FXL6408_Output[address - 0x43], port, false);
    returnValues += ioAccess_FXL6408_setByteI2C(address, 0x05, ioAccess_FXL6408_Output[address - 0x43]);
    // disable High-Z
    returnValues += ioAccess_FXL6408_setByteI2C(address, 0x07, setBit(ioAccess_FXL6408_getByteI2C(address, 0x07), port, false));
    // direction
    returnValues += ioAccess_FXL6408_setByteI2C(address, 0x03, setBit(ioAccess_FXL6408_getByteI2C(address, 0x03), port, true));

    return returnValues == 0;
  }

void ioAccess_FXL6408_setDigitalOutput(byte i2cAddress, uint8_t port, bool state) {
  uint8_t oldRegister = ioAccess_FXL6408_Output[i2cAddress - 0x43];
  ioAccess_FXL6408_Output[i2cAddress - 0x43] = setBit(ioAccess_FXL6408_Output[i2cAddress - 0x43], port, state);
  if (oldRegister != ioAccess_FXL6408_Output[i2cAddress - 0x43]) {
    ioAccess_FXL6408_setByteI2C(i2cAddress, 0x05, ioAccess_FXL6408_Output[i2cAddress - 0x43]);
  }
};

bool ioAccess_FXL6408_configureAsDigitalInput(byte i2cAddress, uint8_t port, bool usePullUpDown, bool pullDirectionUp) {
  int returnValues = 0;
  // pullUp/Down
  returnValues += ioAccess_FXL6408_setByteI2C(i2cAddress, 0x0D, setBit(ioAccess_FXL6408_getByteI2C(i2cAddress, 0x0D), port, pullDirectionUp));
  returnValues += ioAccess_FXL6408_setByteI2C(i2cAddress, 0x0B, setBit(ioAccess_FXL6408_getByteI2C(i2cAddress, 0x0B), port, usePullUpDown));
  // direction
  returnValues += ioAccess_FXL6408_setByteI2C(i2cAddress, 0x03, setBit(ioAccess_FXL6408_getByteI2C(i2cAddress, 0x03), port, false));

  return returnValues == 0;
}

bool ioAccess_FXL6408_getDigitalInput(byte i2cAddress, uint8_t port)  {
   uint8_t value = ioAccess_FXL6408_getByteI2C(i2cAddress, 0xF);
   value = (value >> port) & 1; // shift so the port is at last bit, then mask
   return value == 1;
 }

  uint8_t ioAccess_FXL6408_getByteI2C(byte i2cAddress, int i2cregister) {
    uint8_t result;
    if ( xSemaphoreTake( i2cMutex, 1000 ) == pdTRUE ) {
      Wire.beginTransmission(i2cAddress);
      Wire.write(i2cregister);
      Wire.endTransmission(false);
      uint8_t state = Wire.requestFrom(i2cAddress, 1, (int)true);
      result = Wire.read();
      xSemaphoreGive( i2cMutex );
    }
    return result;
  }

  uint8_t ioAccess_FXL6408_setByteI2C(byte i2cAddress, byte i2cregister, byte value) {
      uint8_t result = 255;
      if ( xSemaphoreTake( i2cMutex, 1000 ) == pdTRUE ) {
        Wire.beginTransmission(i2cAddress);
        Wire.write(i2cregister);
        Wire.write(value);
        result = Wire.endTransmission();
        xSemaphoreGive( i2cMutex );
      }
      return result;
    }
