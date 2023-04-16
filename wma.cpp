#include "Arduino.h"
#include "wma.h"
extern const size_t WMA_MAX_WIDTH;

WeightedMovingAverage::WeightedMovingAverage(size_t width) : _width(min(width, WMA_MAX_WIDTH)), _sum(0), _weighted(0), _values{}, _count(0) {}

int16_t WeightedMovingAverage::get_average(uint16_t value) {
  // Calculate sum with current value and remove oldest value
  // Serial.println(_sum);
  int32_t next = _sum + value - _values[_count];
  // Serial.print("next: ");
  // Serial.println(value);
  // Calculate weighted sum with current value
  int32_t next_weighted = _weighted + (value * _width) - _sum;
  // Serial.print("next_weighted: ");
  // Serial.println(value);
  _sum = next;
  _weighted = next_weighted;
  _values[_count] = value;
  _count = (_count + 1) % _width;
  // Serial.print("sum | ");
  // Serial.println(_sum);
  // Serial.print("weighted | ");
  // Serial.println(_weighted);
  // Serial.print("count | ");
  // Serial.println(_count);
  // Sum of weights = sum of integers from [1, _size]
  int16_t sum_weights = ((_width * (_width + 1)) / 2);
  // Serial.print("sum_weights | ");
  // Serial.println(sum_weights);
  // Serial.println();
  return next_weighted / sum_weights;
}
