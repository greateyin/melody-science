// Reads all Nano pins and prints their current states.
// Note: D0/D1 are used by serial and may be noisy.

const int digitalPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const int analogPins[] = {A0, A1, A2, A3, A4, A5, A6, A7};

void setup() {
  Serial.begin(115200);
  // Give the serial port time to open.
  delay(500);

  // Avoid changing D0/D1 to keep Serial stable.
  for (unsigned int i = 0; i < sizeof(digitalPins) / sizeof(digitalPins[0]); i++) {
    int pin = digitalPins[i];
    if (pin == 0 || pin == 1) {
      continue;
    }
    pinMode(pin, INPUT_PULLUP);
  }
}

void loop() {
  Serial.println("-- Digital pins (INPUT_PULLUP) --");
  for (unsigned int i = 0; i < sizeof(digitalPins) / sizeof(digitalPins[0]); i++) {
    int pin = digitalPins[i];
    int val = digitalRead(pin);
    Serial.print("D");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(val == HIGH ? "HIGH" : "LOW");
  }

  Serial.println("-- Analog pins (ADC 0-1023) --");
  for (unsigned int i = 0; i < sizeof(analogPins) / sizeof(analogPins[0]); i++) {
    int pin = analogPins[i];
    int val = analogRead(pin);
    Serial.print("A");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(val);
  }

  Serial.println();
  delay(1000);
}
