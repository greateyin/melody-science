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
  Serial.println("HX711 calibration");
  Serial.println("1) Remove all weight");
  Serial.println("2) Place 50g weight");
  Serial.println("Reading...\n");
}

void loop() {
  long reading = scale.read_average(10);
  Serial.print("raw:");
  Serial.println(reading);
  delay(500);
}
