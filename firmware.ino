/*
  fsr firmware for Arduino Micro

  Author: Matthew Turner
  Last modified: 15 April 2023
*/
#include <inttypes.h>

#include "Keyboard.h"

const size_t MAX_SHARED_SENSORS = 2;
const long BAUD_RATE = 57600;
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

    size_t get_index(uint8_t fsr_id);
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
    Fsr(uint8_t pin, FsrState* state = nullptr);
    ~Fsr();
    void init(uint8_t id);
    void add_fsr(uint8_t fsr_id);
    void eval_fsr(uint8_t fsr_id, uint16_t value, uint16_t threshold);
    size_t get_index(uint8_t fsr_id);
  private:
    bool _initialized;
    uint8_t _ids[];
    size_t _count;
    enum State { Off, On };
    State _states[];
    State _combined_state = On;
    const uint16_t PADDING_WIDTH;
    uint8_t button_id;
};

/*** END Fsr Class ***/

void setup() {
  Serial.begin(BAUD_RATE);

  input_start();
}

void loop() {
  
}
