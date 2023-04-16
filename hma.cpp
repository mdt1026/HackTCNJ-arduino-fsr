#include <inttypes.h>
#include "hma.h"

HullMovingAverage::HullMovingAverage(size_t size) : _wma1(size/2), _wma2(size), _hull(sqrt(size)) {}

uint16_t HullMovingAverage::get_average(uint16_t value) {
  uint16_t wma1_value = _wma1.get_average(value);
  uint16_t wma2_value = _wma2.get_average(value);
  uint16_t hull_value = _hull.get_average(2 * (wma1_value - (wma2_value/2)));
  return hull_value;
}
