#ifndef WMA_H
#define WMA_H
#include <inttypes.h>
#include "fsr_globals.h"

#ifndef WMA_WIDTH
#define WMA_WIDTH
const size_t WMA_MAX_WIDTH = 50;
#endif

class WeightedMovingAverage {
  public:
    WeightedMovingAverage() = delete;
    WeightedMovingAverage(size_t);
    int16_t get_average(uint16_t);
  private:
    size_t _count;
    size_t _width;
    int32_t _sum;
    int32_t _weighted;
    int16_t _values[WMA_MAX_WIDTH];
};

#endif
