#include "wma.h"
extern const size_t WMA_MAX_WIDTH;

WeightedMovingAverage::WeightedMovingAverage(size_t width) : _width(min(width, WMA_MAX_WIDTH)), _sum(0), _weighted(0), _values{}, _count(0) {}

uint16_t WeightedMovingAverage::get_average(uint16_t value) {
  // Calculate sum with current value and remove oldest value
  uint32_t next = _sum - _values[_count] + value;
  // Calculate weighted sum with current value
  uint32_t next_weighted = _weighted - _sum + (value * _width);
  _sum = next;
  _weighted = next_weighted;
  _values[_count] = value;
  _count = (_count + 1) % _width;
  // Sum of weights = sum of integers from [1, _size]
  uint16_t sum_weights = ((_width* (_width + 1)) / 2);
  return next_weighted / sum_weights;
}
