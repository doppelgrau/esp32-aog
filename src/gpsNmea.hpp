#ifndef gpsNmea_HPP
#define gpsNmea_HPP


struct GpsNmeaOutput {
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
extern GpsNmeaOutput gpsNmeaOutput;

#endif
