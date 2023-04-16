#include <inttypes.h>
#include "Arduino.h"
#include "hma.h"

HullMovingAverage::HullMovingAverage(size_t size) : _wma1(size/2), _wma2(size), _hull(sqrt(size)) {}

int16_t HullMovingAverage::get_average(uint16_t value) {
  int16_t wma1_value = _wma1.get_average(value);
  int16_t wma2_value = _wma2.get_average(value);
  int16_t hull_value = _hull.get_average(max(2 * wma1_value - wma2_value, 0));
  // Serial.println(hull_value);
  // return hull_value;
  return constrain(hull_value, 0, 1023);
}
