#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS A0

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.print("Temp C: ");
  Serial.println(tempC, 2);
  delay(1000);
}
