#include "gpsCommon.hpp"

#ifndef gpsRtcm_HPP
#define gpsRtcm_HPP

void gpsRtcmSetup(GpsRtcmData::RtcmDestination rtcmdestination);
void gpsRtcmCreateUdpReceiveHandler();
void gpsRtcmBtReceiver( void* z );
void gpsRtcmNtripReceiver( void* z );

#endif
