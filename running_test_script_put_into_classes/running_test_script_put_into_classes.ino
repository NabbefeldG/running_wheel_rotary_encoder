#include "Arduino.h"
#include "Wheel_counter.h"


// Module Info
#define FirmwareVersion "0001" // This doesnt mean anything here I would say, just copied from TouchShaker
#define moduleName "Running_wheel_module" // Name of module for manual override UI and state machine assembler


// Signnal Bytes
#define GOT_BYTE 14
#define DID_ABORT 15
#define DATA_REQUEST 1
#define RESET_COUNTERS 2
#define MODULE_INFO 255  // returns module information


// Wheel 1 Inputs
const byte Wheel1PinA = 12;
const byte Wheel1PinB = 14;
//bool Wheel1LastState;
//bool Wheel1State;
//bool Wheel1cw_direction = 0;
//unsigned long Wheel1_cw_counter = 0;  // clockwise
//unsigned long Wheel1_ccw_counter = 0;  // counter clockwise

// Wheel 2 Inputs
const byte Wheel2PinA = 27;
const byte Wheel2PinB = 26;
//bool Wheel2LastState;
//bool Wheel2State;
//bool Wheel2cw_direction = 0;
//unsigned long Wheel2_cw_counter = 0;  // clockwise
//unsigned long Wheel2_ccw_counter = 0;  // counter clockwise


// Init wheel classes
Wheel_counter wheel_counter1(Wheel1PinA, Wheel1PinB);
Wheel_counter wheel_counter2(Wheel2PinA, Wheel2PinB);


// Serial COM variables
unsigned long serialClocker = millis();
int FSMheader = 0;
bool midRead = 0;
bool read_msg_length = 0;
float temp[2]; // temporary variable for general purposes
bool data_requested = true;


// Variables
unsigned long last_usb_update_clock = millis();
unsigned long usb_timeout = 10;  // in ms


// I've tried to make these just a part of the class, but apparently there is just no way of defining the interrupt inside the class. I only found suggestes that made it far more complicated than this:/
void wheel1interrupt() { wheel_counter1._interrupt_call(); }
void wheel2interrupt() { wheel_counter2._interrupt_call(); }

void setup() {
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(wheel_counter1._pinA), wheel1interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(wheel_counter2._pinA), wheel2interrupt, CHANGE);
}

void loop() {
  ReadSerialCommunication();

  // Send data to PC
  if (data_requested) {
    delay(5);
    String msg = "Time in ms:";
    Serial.println(msg + String(millis()) + ";" + String(wheel_counter1.cw_counter) + ";" + String(wheel_counter1.ccw_counter) + 
                                            ";" + String(wheel_counter2.cw_counter) + ";" + String(wheel_counter2.ccw_counter));
    data_requested = false;
  }
}

void ReadSerialCommunication() {
  // Serial Communication over USB
  if (Serial.available() > 0) {
    if (!midRead) {
      FSMheader = Serial.read();
      midRead = 1; // flag for current reading of serial information
      serialClocker = millis(); // counter to make sure that all serial information arrives within a reasonable time frame (currently 100ms)
      read_msg_length = false;
    }
    
    if (midRead) {
      if (FSMheader == DATA_REQUEST) {
        Serial.write(GOT_BYTE); midRead = 0;
        data_requested = true;
      }
      else if (FSMheader == RESET_COUNTERS) {
        Serial.write(GOT_BYTE); midRead = 0;
        wheel_counter1.reset_counters();
        wheel_counter2.reset_counters();
      }
      else if (FSMheader == MODULE_INFO) { // return module information to bpod
        returnModuleInfo(); midRead = 0;
      }
      
      else {
        //flush the Serial to be ready for new data
        while (Serial.available() > 0) {
          Serial.read();
        }
  
        //send abort to request resend
        Serial.write(DID_ABORT); midRead = 0;
      }
    }
  }

  if (midRead && ((millis() - serialClocker) >= 100)) {
      //flush the Serial to be ready for new data
      while (Serial.available() > 0) {
        Serial.read();
      }

      //send abort to request resend
      Serial.write(DID_ABORT); midRead = 0;
  }
}

float readSerialChar(byte currentRead){
  float currentVar = 0;
  byte cBytes[currentRead-1]; // current byte
  int preDot = currentRead; // indicates how many characters there are before a dot
  int cnt = 0; // character counter

  if (currentRead == 1){
    currentVar = Serial.read() -'0'; 
  }

  else {
    for (int i = 0; i < currentRead; i++) {
      cBytes[i] = Serial.read(); // go through all characters and check for dot or non-numeric characters
      if (cBytes[i] == '.') {cBytes[i] = '0'; preDot = i;}
      if (cBytes[i] < '0') {cBytes[i] = '0';}
      if (cBytes[i] > '9') {cBytes[i] = '9';}
    }
  
    // go through all characters to create new number
    if (currentRead > 1) {
      for (int i = preDot-1; i >= 1; i--) {
        currentVar = currentVar + ((cBytes[cnt]-'0') * pow(10,i));
        cnt++;
      }
    }
    currentVar = currentVar + (cBytes[cnt] -'0'); 
    cnt++;
  
    // add numbers after the dot
    if (preDot != currentRead){
      for (int i = 0; i < (currentRead-preDot); i++) {
        currentVar = currentVar + ((cBytes[cnt]-'0') / pow(10,i));
        cnt++;
      }
    }
  }
  return currentVar;
}

void returnModuleInfo() {
  Serial.write(65); // Acknowledge
  Serial.write(FirmwareVersion); // 4-byte firmware version
  Serial.write(sizeof(moduleName)-1); // Length of module name
  Serial.print(moduleName); // Module name
  Serial.write(0); // 1 if more info follows, 0 if not
}
