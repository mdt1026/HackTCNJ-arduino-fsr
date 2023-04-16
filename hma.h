#ifndef HMA_H
#define HMA_H
#include <inttypes.h>
#include "fsr_globals.h"
#include "wma.h"

class HullMovingAverage {
  public:
    HullMovingAverage() = delete;
    HullMovingAverage(size_t);
    uint16_t get_average(uint16_t);
  private:
    WeightedMovingAverage _wma1;
    WeightedMovingAverage _wma2;
    WeightedMovingAverage _hull;
};

#endif
