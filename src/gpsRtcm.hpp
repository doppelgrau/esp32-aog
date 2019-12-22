#include <BluetoothSerial.h>


#ifndef gpsRtcm_HPP
#define gpsRtcm_HPP

extern BluetoothSerial gpsCommonBtSerial;

struct GpsRtcmData {
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
extern GpsRtcmData gpsRtcmData;

void gpsRtcmSetup(GpsRtcmData::RtcmDestination rtcmdestination);
void gpsRtcmCreateUdpReceiveHandler();
void gpsRtcmBtReceiver( void* z );
void gpsRtcmNtripReceiver( void* z );

#endif
