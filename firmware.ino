/*
  fsr firmware for Arduino Micro

  Author: Matthew Turner
  Last modified: 15 April 2023
*/
#include <inttypes.h>

#include "Keyboard.h"

const long BAUD_RATE = 57600;
const size_t MAX_SHARED_SENSORS = 2;
const uint16_t DEFAULT_THRESHOLD = 1000;

uint8_t current_button = 1;


/*** BEGIN Keyboard | Joystick Functions ***/
void input_start() {
  Keyboard.begin();
}
void input_press(uint8_t id) {
  Keyboard.press('a' + id - 1);
}
void input_release(uint8_t id) {
  Keyboard.press('a' + id - 1);
}

/*** END Keyboard | Joystick Functions ***/


/*** BEGIN FsrState Class ***/

// Define state class for managing a set of sensors
// and triggering button press and release events
class FsrState {
  public:
    FsrState() : _count(0), button_id(current_button++) {
      for (size_t i=0; i < MAX_SHARED_SENSORS; i++) {
        _ids[i] = 0;
        _states[i] = FsrState::Off;
      }
    }

    void init() {
      if (_initialized) { return; }
      button_id = current_button++;
      _initialized = true;
    }

    // Adds new fsr to this state
    // If we have max sensors in this state,
    // then the new one is dropped instead of being added.
    void add_fsr(uint8_t fsr_id) {
      if (_count < MAX_SHARED_SENSORS ) { _ids[_count++] = fsr_id; }
    }

    // Evaluates an fsr as part of the shared state.
    void eval_fsr(uint8_t fsr_id, uint16_t value, uint16_t threshold) {
      if (!_initialized) { return; }

      size_t index;
      if ((index = get_index(fsr_id)) < 0) { return; }

      // Turn sensor on if equal to or above the threshold
      // Turn it off if below the threshold
      if (value >= threshold + PADDING_WIDTH) { _states[index] = FsrState::On; }
      else if (value < threshold - PADDING_WIDTH) { _states[index] = FsrState::Off; }

      if (index == _count - 1) {
        switch (_combined_state) {
        case FsrState::Off:
          {
            bool enable = false;
            for (size_t i=0; i < _count; i++) {
              if (_states[i] == FsrState::On) {
                enable = true;
                break;
              }
            }
            if (enable) {
              input_press(button_id);
              _combined_state = FsrState::On;
            }
          }
          break;
        case FsrState::On:
          {
            bool disable = true;
            for (size_t i=0; i < _count; i++) {
              if (_states[i] == FsrState::On) {
                disable = false;
                break;
              }
            }
            if (disable) {
              input_release(button_id);
              _combined_state = FsrState::Off;
            }
          }
          break;
        }
      }
    }

    size_t get_index(uint8_t fsr_id) {
      for (size_t i = 0; i < _count; i++) {
        if (_ids[i] == fsr_id) {
          return i;
        }
      }
      return -1;
    }

  private:
    const uint16_t PADDING_WIDTH = 1;
    enum State { Off, On };

    bool _initialized;
    size_t _count;
    uint8_t button_id;
    State _combined_state = FsrState::On;

    uint8_t _ids[];
    State _states[];
};

/*** END FsrState Class ***/


/*** BEGIN Fsr Class ***/

// Define class for each sensor's information
class Fsr {
  public:
    // TODO Add moving_average to initialization list
    Fsr(uint8_t pin, FsrState* state = nullptr) : _initialized(false), _pin(pin), _threshold(DEFAULT_THRESHOLD), _offset(0), _state_owner(false) {}

    ~Fsr() { if (_state_owner) { delete _state; } }

    void init(uint8_t id) {
      if (_initialized) { return; }
      if (!id) { return; }

      if (_state == nullptr) {
        _state = new FsrState();
        _state_owner = true;
      }

      _state->init();

      if (_state->get_index(id) < 0) { _state->add_fsr(id); }
      _fsr_id = id;
      _initialized = true;
    }

    void eval_fsr(bool will_send) {
      if (!_initialized) { return; }
      if (_state->get_index(_fsr_id) < 0) { return; }

      int16_t value = analogRead(_pin);
      // TODO Implement Moving Average smoothing on sensor value
      _value = value;


      if (will_send) { _state->eval_fsr(_fsr_id, _value, _threshold); }
    }

    void set_threshold(uint16_t threshold) { _threshold = threshold; }
    uint16_t get_threshold() { return _threshold; }

    // Set offset to last read value and return offset
    // NOTE: Should be called with no applied pressure for calibration.
    uint16_t set_offset() {
      _offset = _value;
      return _offset;
    }

    uint16_t get_value() { return _value; }

    // Remove the default constructor so pin must be given
    Fsr() = delete;

  private:
    bool _initialized;
    bool _state_owner;

    uint8_t _fsr_id;
    uint8_t _pin;
    uint16_t _threshold;
    uint16_t _value;
    uint16_t _offset;

    // TODO HullMovingAverage moving_average_
    FsrState* _state;
};

/*** END Fsr Class ***/

void setup() {
  Serial.begin(BAUD_RATE);

  input_start();
}

void loop() {
  
}
