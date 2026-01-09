#include <math.h>

#define NTC_PIN A0
#define R_FIXED 10000.0      // 10kΩ
#define R0 100000.0          // NTC 100k
#define BETA 3950.0
#define T0 298.15            // 25°C (K)

float readNTC() {
  int adc = analogRead(NTC_PIN);
  if (adc <= 0) {
    return NAN;
  }
  float R = R_FIXED * (1023.0 / adc - 1.0);
  float invT = (1.0 / T0) + (1.0 / BETA) * log(R / R0);
  return (1.0 / invT) - 273.15;
}

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("NTC test: adc,temp_C");
}

void loop() {
  int adc = analogRead(NTC_PIN);
  float tempC = readNTC();
  Serial.print(adc);
  Serial.print(",");
  if (isnan(tempC)) {
    Serial.println("nan");
  } else {
    Serial.println(tempC, 2);
  }
  delay(1000);
}
