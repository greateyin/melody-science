#include "HX711.h"

#define HX_DT 6
#define HX_SCK 7

HX711 scale;

void setup() {
  Serial.begin(9600);
  delay(500);

  scale.begin(HX_DT, HX_SCK);
  scale.set_scale();
  scale.tare();

  Serial.println("HX711-only: raw(avg10)");
}

void loop() {
  long raw = scale.read_average(10);
  Serial.println(raw);
  delay(500);
}
