#include <BluetoothSerial.h>


#ifndef gpsCommon_HPP
#define gpsCommon_HPP

extern BluetoothSerial gpsCommonBtSerial;

struct GpsCommonRtcm {
  enum class RtcmSource : uint8_t {
    none = 0,
    UDP = 1,
    BlueTooth = 2,
    Ntrip = 3
  } rtcmSource = RtcmSource::none;

  enum class RtcmStatus : uint8_t {
    setup = 0,
    working = 1,
    error = 2,
  } rtcmStatus = RtcmStatus::setup;



  enum class RtcmDestination : uint8_t {
    gps1 = 0,
    gps2 = 1,
    both = 2,
  } rtcmDestination = RtcmDestination::gps1;
  uint lastReceived = 0;
};
extern GpsCommonRtcm gpsCommonRtcm;


struct GpsCommonNmeaOutput {
  bool udpOutput = false;
  bool btOutput = false;
  bool serialOutput = false;

  uint lastSent = 0;
  enum class GpsQuality : uint8_t {
    none = 0,
    gps = 1,
    dgps = 2,
    floatrtk = 5,
    fixedrtk = 4,
  } qpsquality = GpsQuality::none;

  String lastGGA;
};
extern GpsCommonNmeaOutput gpsCommonNmeaOutput;

void gpsCommonCreateRtcmReceiveHandler();
void gpsCommonBtRtcmReceiver( void* z );
void gpsCommonNtripReceiver( void* z );

#endif
