#include "Arduino.h"
#include "Wheel_counter.h"

// SD Module Libs
#include "FS.h"
#include "SD.h"
#include "SPI.h"


// Wheel 1 Inputs
const byte Wheel1PinA = 27;
const byte Wheel1PinB = 26;

// Wheel 2 Inputs
const byte Wheel2PinA = 25;
const byte Wheel2PinB = 33;


// Init wheel classes
Wheel_counter wheel_counter1(Wheel1PinA, Wheel1PinB);
Wheel_counter wheel_counter2(Wheel2PinA, Wheel2PinB);


// Variables
unsigned long last_data_request = millis();


// I've tried to make these just a part of the class, but apparently there is just no way of defining the interrupt inside the class. I only found suggestes that made it far more complicated than this:/
void wheel1interrupt() { wheel_counter1._interrupt_call(); }
void wheel2interrupt() { wheel_counter2._interrupt_call(); }

// Handling SD related stuff
// char file_name[29] = "\debug_file_0000000000000.txt";
char file_name[] = "/data000000.txt";


void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  blink_LED_error();

  SD.begin();

  int Cnt = 0;
  while (SD.exists(file_name)) {
    Cnt += 1;
    // Simply count up files names. It would be nice to have more elaborat file names, but this would require a rt-clock. 
    // In an earlier version I used serial to communicate with the PC. I could just send the file name then, but when we switched to the SD card this became an issue.
    sprintf(file_name, "/data%06d.txt", Cnt);
  }

  // Create Data file
  String header_str = "Time in ms;Wheel1_cw_count;Wheel1_ccw_count;Wheel2_cw_count;Wheel2_ccw_count\n";
  writeFile(SD, file_name, header_str.c_str());

  if (!SD.exists(file_name)) {
    // This means that creating the file failed!
    while (1) {
      // Make it clear that something is wrong!
      blink_LED_error();
    }
  }

  attachInterrupt(digitalPinToInterrupt(wheel_counter1._pinA), wheel1interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(wheel_counter2._pinA), wheel2interrupt, CHANGE);

  // Indicate that the setup routine worked
  digitalWrite(13, HIGH);
}

void loop() {
  // Log the current rotation counts every 5s. This was the resired temporal resolution for us, but can be changed if desired.  
  if (millis() - last_data_request >= 5000) {
    String msg = String(millis()) + ";" + String(wheel_counter1.cw_counter) + ";" + String(wheel_counter1.ccw_counter) + 
                                    ";" + String(wheel_counter2.cw_counter) + ";" + String(wheel_counter2.ccw_counter) + "\n";

    // save to SD card
    appendFile(SD, file_name, msg.c_str());

    // Feedback for the user that data was logged, just in case
    digitalWrite(13, LOW);
    delay(500);
    digitalWrite(13, HIGH);

    // reset timer
    last_data_request = millis();
  }
}


// FUNCTIONS FROM HERE ON: //
void writeFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    blink_LED_error();
    return;
  }
  file.print(message);
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_APPEND);
  if(!file){
    blink_LED_error();
    return;
  }
  // Write and ignore if it fails
  if (!file.print(message)) {
    blink_LED_error();
  }
  file.close();
}

void blink_LED_error() {
  for (int j = 0; j < 16; j++) {
    digitalWrite(13, LOW);
    delay(50);
    digitalWrite(13, HIGH);
    delay(50);
  }
}

