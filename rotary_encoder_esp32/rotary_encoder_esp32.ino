#include "Arduino.h"
#include "Wheel_counter.h"

// SD Module Libs
#include "FS.h"
#include "SD.h"
#include "SPI.h"



// Module Info
#define FirmwareVersion "0001" // This doesnt mean anything here I would say, just copied from TouchShaker
#define moduleName "rotary_encoder_esp32" // Name of module for manual override UI and state machine assembler


// Signnal Bytes
#define GOT_BYTE 14
#define DID_ABORT 15
#define DATA_REQUEST 1
#define RESET_COUNTERS 2
#define SET_FILE_NAME 3
#define MODULE_INFO 255  // returns module information


// Wheel 1 Inputs
const byte Wheel1PinA = 12;
const byte Wheel1PinB = 14;

// Wheel 2 Inputs
const byte Wheel2PinA = 27;
const byte Wheel2PinB = 26;


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
unsigned long last_data_request = millis();
unsigned long last_usb_update_clock = millis();
// unsigned long usb_timeout = 10;  // in ms


// I've tried to make these just a part of the class, but apparently there is just no way of defining the interrupt inside the class. I only found suggestes that made it far more complicated than this:/
void wheel1interrupt() { wheel_counter1._interrupt_call(); }
void wheel2interrupt() { wheel_counter2._interrupt_call(); }

// Handling SD related stuff
bool session_started = false;
bool PC_timeout = false;
char file_name[28];


void setup() {
  SD.begin();
//  uint8_t cardType = SD.cardType();
//  if(cardType == CARD_NONE){ return; }
  
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(wheel_counter1._pinA), wheel1interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(wheel_counter2._pinA), wheel2interrupt, CHANGE);
}

void loop() {
  // This is code provided by Simon Musall from the Churchland lab
  ReadSerialCommunication();

  // Send data to PC
  if (session_started) {
    if (data_requested || (PC_timeout && (millis() - last_data_request > 5000))) {
      delay(5);
      String msg = "Time in ms:" + String(millis()) + ";" + String(wheel_counter1.cw_counter) + ";" + String(wheel_counter1.ccw_counter) + 
                                              ";" + String(wheel_counter2.cw_counter) + ";" + String(wheel_counter2.ccw_counter) + "\n";
  //    String msg = "Time in ms:";
  //    Serial.println(msg + String(millis()) + ";" + String(wheel_counter1.cw_counter) + ";" + String(wheel_counter1.ccw_counter) + 
  //                                            ";" + String(wheel_counter2.cw_counter) + ";" + String(wheel_counter2.ccw_counter));
      // Send to PC
      Serial.println(msg);

      // Also save to SD
      appendFile(SD, file_name, msg.c_str());
  
      data_requested = false;
      last_data_request = millis();

      if (PC_timeout && data_requested) {
        // I Don't think this will occure, its just for debugging.
        String debug_msg = "PC_timeout && data_requested\n";
        appendFile(SD, file_name, debug_msg.c_str());
      }
    }
  }

  // Check if its been longer than 10s. If so I assume that the PC got disconnected and I switch to saving autonomously ever 5s
  if (millis() - last_data_request > 10000) {
    PC_timeout = true;
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
      else if (FSMheader == SET_FILE_NAME) {
        // I don't care here if this stalls the rest of the script
        Serial.write(GOT_BYTE); midRead = 0;
        while (Serial.available() < 27) {
          delay(1);
          if (millis() - serialClocker >= 100) {
            break;
          }
        }

        file_name[0] = '/';
        if (Serial.available() >= 27) {
          for (int i = 0; i < 27; i++) { // read the individual chars of the file name
            file_name[i+1] = Serial.read();
          }
        }

        String header_str = "Time in ms;Wheel1_cw_count;Wheel1_ccw_count;Wheel2_cw_count;Wheel2_ccw_count\n";
        writeFile(SD, file_name, header_str.c_str());

        last_data_request = millis();
        session_started = true;
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


// FUNCTION FROM HERE ON: //
void writeFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    return;
  }
  file.print(message);
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_APPEND);
  if(!file){
    return;
  }
  // Write and ignore if it fails
  file.print(message);
  file.close();
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
