#include "Arduino.h"


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
bool Wheel1LastState;
bool Wheel1State;
bool Wheel1cw_direction = 0;
unsigned long Wheel1_cw_counter = 0;  // clockwise
unsigned long Wheel1_ccw_counter = 0;  // counter clockwise

// Wheel 2 Inputs
const byte Wheel2PinA = 27;
const byte Wheel2PinB = 26;
bool Wheel2LastState;
bool Wheel2State;
bool Wheel2cw_direction = 0;
unsigned long Wheel2_cw_counter = 0;  // clockwise
unsigned long Wheel2_ccw_counter = 0;  // counter clockwise

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


bool A;
bool B;


void setup() {
  Serial.begin(115200);
  pinMode(Wheel1PinA, INPUT);
  pinMode(Wheel1PinB, INPUT);
  attachInterrupt(digitalPinToInterrupt(Wheel1PinA), Wheel1tick, CHANGE);
  
  pinMode(Wheel2PinA, INPUT);
  pinMode(Wheel2PinB, INPUT);
  // attachInterrupt(digitalPinToInterrupt(Wheel2PinA), Wheel2tick, CHANGE);

  // A = digitalRead(Wheel1PinA);
  // B = digitalRead(Wheel1PinB);
  Wheel1LastState = digitalRead(Wheel1PinA);
  Wheel1State = Wheel1LastState;
  // Wheel2LastState = digitalRead(Wheel2PinA);
  // Wheel2State = Wheel2LastState;
}

void loop() {
  ReadSerialCommunication();

  // Send data to PC
  if (data_requested) {
    delay(5);
    String msg = "Time in ms:";
    Serial.println(msg + String(millis()) + ";DataCW:" + String(Wheel1_cw_counter) + ";DataCCW:" + String(Wheel1_ccw_counter));
    data_requested = false;
  }
}

void Wheel1tick() {
  Wheel1State = digitalRead(Wheel1PinA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (Wheel1State != Wheel1LastState){
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(Wheel1PinB) != Wheel1State) {
      if (Wheel1cw_direction) {
        Wheel1_ccw_counter ++;
      } else {
        Wheel1cw_direction = 1;
      }
    } else {
      if (cw_direction) {
        Wheel1cw_direction = 0;
      } else {
        Wheel1_cw_counter ++;
      }
    }
    Wheel1LastState = Wheel1State; // Updates the previous state of the outputA with the current state
  }
}

void Wheel2tick() {
  Wheel2State = digitalRead(Wheel2PinA); // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (Wheel2State != Wheel2LastState){
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(Wheel2PinB) != Wheel2State) {
      if (Wheel2cw_direction) {
        Wheel12ccw_counter ++;
      } else {
        Wheel2cw_direction = 1;
      }
    } else {
      if (Wheel2cw_direction) {
        Wheel2cw_direction = 0;
      } else {
        Wheel2_cw_counter ++;
      }
    }
    Wheel2LastState = Wheel2State; // Updates the previous state of the outputA with the current state
  }
}

void ReadSerialCommunication() {
  // Serial Communication over USB
  // Main purpose is to start a new trial
  if (Serial.available() > 0) {
    if (!midRead) {
      FSMheader = Serial.read();
      midRead = 1; // flag for current reading of serial information
      serialClocker = millis(); // counter to make sure that all serial information arrives within a reasonable time frame (currently 100ms)
      read_msg_length = false;
    }
    
    if (midRead) {
      if (FSMheader == DATA_REQUEST) {
        // TODO: Request Data and respond by sending
        Serial.write(GOT_BYTE); midRead = 0;
        data_requested = true;
      }
      else if (FSMheader == RESET_COUNTERS) {
        // TODO: reset counters
        Serial.write(GOT_BYTE); midRead = 0;
        Wheel1_cw_counter = 0;
        Wheel1_ccw_counter = 0;
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
