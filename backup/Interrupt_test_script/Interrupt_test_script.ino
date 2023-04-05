// const byte ledPin = 12;
const byte interruptPin = 12;
volatile byte state = LOW;

// I renambed "blink" to "change_state"

void setup() {
  Serial.begin(9600);
  // pinMode(ledPin, OUTPUT);
  // pinMode(interruptPin, INPUT_PULLUP);
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), change_state, CHANGE);
}

void loop() {
  delay(1000);
  // digitalWrite(ledPin, state);
}

void change_state() {
  Serial.println("Pulse");
  // state = !state;
}
