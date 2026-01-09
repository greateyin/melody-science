#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define OLED_ADDR 0x3C

Adafruit_SH1106G display(128, 64, &Wire, -1);

void setup() {
  Wire.begin();
  Serial.begin(9600);

  display.begin(OLED_ADDR, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("SH1106 128x64");
  display.println("I2C 0x3C");
  display.println("Test OK");
  display.display();
}

void loop() {
}
