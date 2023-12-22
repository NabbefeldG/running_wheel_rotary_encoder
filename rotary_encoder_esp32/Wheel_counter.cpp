#include "Arduino.h"
#include "Wheel_counter.h"

Wheel_counter::Wheel_counter(byte PinA, byte PinB) {
  // PINs
  _pinA = PinA; // Define Inputs
  _pinB = PinB; // Define Inputs

  // Initialize PINs
  pinMode(_pinA, INPUT);
  pinMode(_pinB, INPUT);

  _state = digitalRead(_pinA);
  _lastState = _state;
  _direction = digitalRead(_pinB) != _state;  // Init direction according to the schema in Interrupt_call()
}

void Wheel_counter::_interrupt_call() {
  _state = digitalRead(_pinA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  // This is an obsolete call since I made this an interrupt, but it also doesn't harm so I leave this condition of now
  if (_state != _lastState){
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(_pinB) != _state) {
      if (_direction) {
        ccw_counter ++;
      } else {
        _direction = 1;
      }
    } else {
      if (_direction) {
        _direction = 0;
      } else {
        cw_counter ++;
      }
    }
    _lastState = _state; // Updates the previous state of the outputA with the current state
  }
}

void Wheel_counter::reset_counters() {
  cw_counter = 0;
  ccw_counter = 0;
}
