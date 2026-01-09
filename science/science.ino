#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "HX711.h"

#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define ONE_WIRE_BUS A0

// HX711 load cell
#define HX_DT 6
#define HX_SCK 7
float CAL_FACTOR = 903.2; // calibrated with 50g

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
HX711 scale;
bool hxTared = false;

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println("Boot");

  Wire.begin();
  Serial.println("Wire begin");

  // Retry LCD init to avoid occasional blank screen on power-up.
  for (int i = 0; i < 3; i++) {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD init...");
    Serial.print("LCD init pass ");
    Serial.println(i + 1);
    delay(200);
  }

  tempSensors.begin();
  Serial.println("DS18B20 begin");

  scale.begin(HX_DT, HX_SCK);
  scale.set_scale(CAL_FACTOR);
  if (scale.is_ready()) {
    scale.tare();
    hxTared = true;
    Serial.println("HX711 tare OK");
  } else {
    Serial.println("HX711 not ready, skip tare");
  }
  Serial.println("HX711 begin");
}

void loop() {
  tempSensors.requestTemperatures();
  float tempC = tempSensors.getTempCByIndex(0);
  bool tempValid = (tempC > -100.0 && tempC < 150.0);

  bool hxReady = scale.is_ready();
  float weight_g = 0.0;
  if (hxReady) {
    if (!hxTared) {
      scale.tare();
      hxTared = true;
    }
    weight_g = scale.get_units(10);
  }

  Serial.print("tempC=");
  Serial.print(tempC, 2);
  Serial.print(" valid=");
  Serial.print(tempValid ? "Y" : "N");
  Serial.print(" hxReady=");
  Serial.println(hxReady ? "Y" : "N");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (tempValid) {
    lcd.print(tempC, 1);
    lcd.print("C");
  } else {
    lcd.print("--.-C");
  }

  lcd.setCursor(0, 1);
  lcd.print("W:");
  if (hxReady) {
    lcd.print(weight_g, 1);
    lcd.print("g");
  } else {
    lcd.print("--.--");
  }

  delay(1000);
}
