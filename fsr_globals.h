#ifndef FSR_CONSTANTS_H
#define FSR_CONSTANTS_H
#include <inttypes.h>
#include "Arduino.h"

const long BAUD_RATE = 57600;
const uint16_t DEFAULT_THRESHOLD = 50;
const size_t MAX_SHARED_SENSORS = 2;
extern const size_t NUM_FSRS;
extern const size_t WMA_MAX_WIDTH;

extern uint8_t current_button;

#endif
