#ifndef Wheel_counter_h
#define Wheel_counter_h

#include "Arduino.h"

class Wheel_counter {
  public:
    // constructor
    Wheel_counter(byte PinA, byte PinB);

    // call function
    void ReadCounters();  // read counters and transmit to PC for save

    // resets the counters for a new recording
    void reset_counters();

    // I made the Counters accessible from the outside. The alternative would have been to make a read function.
    unsigned int cw_counter = 0;
    unsigned long ccw_counter = 0;

    // Interrupt function
    void _interrupt_call();

    byte _pinA;  // trigger count
    byte _pinB;  // ref. signal
  private:
    // state vars
    bool _state = 0;
    bool _lastState = 0;
    bool _direction = 0;
};

#endif
