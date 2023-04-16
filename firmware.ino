/*
  fsr firmware for Arduino Micro

  Author: Matthew Turner
  Last modified: 15 April 2023
*/
#include <inttypes.h>

#include "fsr_globals.h"
#include "hma.h"

#if defined(_SFR_BYTE) && defined(_BV) && defined(ADCSRA)
  #define CLEAR_BIT(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
  #define SET_BIT(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


/*** BEGIN Keyboard | Joystick Functions ***/
#include "Keyboard.h"

void input_start() {
  Keyboard.begin();
}
void input_press(uint8_t id) {
  Keyboard.press('a' + id - 1);
}
void input_release(uint8_t id) {
  Keyboard.release('a' + id - 1);
}

/*** END Keyboard | Joystick Functions ***/

// extern const size_t WMA_MAX_WIDTH;

uint8_t current_button = 1;


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
      if ((index = get_index(fsr_id)) == -1) { return; }

      // Turn sensor on if equal to or above the threshold
      // Turn it off if below the threshold
      if (value >= threshold + PADDING_WIDTH) { 
        _states[index] = FsrState::On;
      }
      else if (value < threshold - PADDING_WIDTH) {
        _states[index] = FsrState::Off;
      }

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
    State _combined_state = FsrState::Off;

    uint8_t _ids[MAX_SHARED_SENSORS];
    State _states[MAX_SHARED_SENSORS];
};

/*** END FsrState Class ***/


/*** BEGIN Fsr Class ***/

// Define class for each sensor's information
class Fsr {
  public:
    // TODO Add moving_average to initialization list
    Fsr(uint8_t pin, FsrState* state = nullptr) : _initialized(false), _pin(pin), _threshold(DEFAULT_THRESHOLD), _hma(WMA_MAX_WIDTH), _offset(0), _state_owner(false) {}

    ~Fsr() { if (_state_owner) { delete _state; } }

    void init(uint8_t id) {
      if (_initialized) { return; }
      if (!id) { return; }

      if (_state == nullptr) {
        _state = new FsrState();
        _state_owner = true;
      }

      _state->init();

      if (_state->get_index(id) == -1) { _state->add_fsr(id); }
      _fsr_id = id;
      _initialized = true;
    }

    void eval_fsr(bool will_send) {
      if (!_initialized) { return; }
      if (_state->get_index(_fsr_id) == -1) { return; }

      uint16_t value = analogRead(_pin);
      // Serial.print(_fsr_id);
      // Serial.print(" ");
      // Serial.println(value);

      // TODO Implement Moving Average smoothing on sensor value
      // _value = _hma.get_average(value) - _offset;
      _value = constrain(value - _offset, 0, 1023);

      // Serial.print(">> ");
      // Serial.println(_value);
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

    HullMovingAverage _hma;
    FsrState* _state;
};

/*** END Fsr Class ***/

/* Define the sensors and set their pins */
// A3  | Left
// A8  | Down
// A0  | Up
// A11 | Right
Fsr fsrs[] = {
  Fsr(A3),
  Fsr(A8),
  Fsr(A0),
  Fsr(A11),
};
const size_t NUM_FSRS = sizeof(fsrs) / sizeof(Fsr);


/*** BEGIN SerialProcessor Class ***/

class SerialProcessor {
  public:
    void init(long baud_rate) {
      Serial.begin(baud_rate);
      while (!Serial) {}
    }

    void read_data() {
      while (Serial.available() > 0) {
        size_t num_bytes = Serial.readBytesUntil('\n', _buffer, BUFFER_SIZE-1);
        _buffer[num_bytes] = '\0';

        if (num_bytes == 0) { return; }

        switch(_buffer[0]) {
        case 'o':
        case 'O':
          set_offsets();
          break;
        case 'v':
        case 'V':
          print_values();
          break;
        case 't':
        case 'T':
          print_thresholds();
          break;
        case '0' ... '9':
          update_print_thresholds(num_bytes);
        default:
          break;
        }
      }
    }

    void set_offsets() {
      Serial.print("o");
      for (size_t i=0; i < NUM_FSRS; i++) {
        Serial.print(" ");
        Serial.print(fsrs[i].set_offset());
      }
      Serial.print("\n");
    }

    void print_values() {
      Serial.print("v");
      for (size_t i=0; i < NUM_FSRS; i++) {
        Serial.print(" ");
        Serial.print(fsrs[i].get_value());
      }
      Serial.print("\n");
    }

    void print_thresholds() {
      Serial.print("t");
      for (size_t i=0; i < NUM_FSRS; i++) {
        Serial.print(" ");
        Serial.print(fsrs[i].get_threshold());
      }
      Serial.print("\n");
    }

    void update_print_thresholds(size_t num_bytes) {
      // FSR number + Threshold value with a space in between
      // {0, 1, 2, 3, ...} + "0"-"1023"
      // e.g. 3 180 (4th FSR, set threshold to 180
      if (num_bytes < 3 || num_bytes > 7) { return; }

      char* next = nullptr;
      size_t index = strtoul(_buffer, &next, 10);
      if (index >= NUM_FSRS) { return; }

      uint16_t threshold = strtoul(next, nullptr, 10);
      if (threshold > 1023) { return; }

      fsrs[index].set_threshold(threshold);
      print_thresholds();
    }

  private:
    static const size_t BUFFER_SIZE = 64;
    char _buffer[BUFFER_SIZE];
};

/*** END SerialProcessor Class ***/

SerialProcessor serialProcessor;

unsigned long sent_time = 0;
long loop_duration = -1;

void setup() {
  serialProcessor.init(BAUD_RATE);
  input_start();

  for (size_t i=0; i < NUM_FSRS; i++) {
    fsrs[i].init(i+1);
  }

  #if defined(CLEAR_BIT) && defined(SET_BIT)
	  // Set the ADC prescaler to 16 for boards that support it,
	  // which is a good balance between speed and accuracy.  // More information can be found here: http://www.gammon.com.au/adc
	  SET_BIT(ADCSRA, ADPS2);
	  CLEAR_BIT(ADCSRA, ADPS1);
	  CLEAR_BIT(ADCSRA, ADPS0);
  #endif
}

void loop() {
  unsigned long start_microsec = micros();
  static bool will_send;
  will_send = (loop_duration == -1 || start_microsec - sent_time + loop_duration >= 1000);

  serialProcessor.read_data();

  for (size_t i=0; i < NUM_FSRS; i++) {
    fsrs[i].eval_fsr(will_send);
  }

  if (will_send) {
    sent_time = start_microsec;
    // TODO Send Joystick
  }

  if (loop_duration == -1) {
    loop_duration = micros() - start_microsec;
  }

  // delay(500);
}
