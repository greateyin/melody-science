#include <U8g2lib.h>
#include <Wire.h>

// Try SH1106 first (common for 128x64 I2C modules)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  u8g2.begin();
  u8g2.setContrast(255);
}

void loop() {
  u8g2.clearBuffer();
  // Invert: fill white background, then draw black text.
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 64);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 12, "U8g2 SH1106");
  u8g2.drawStr(0, 28, "128x64 I2C");
  u8g2.drawStr(0, 44, "Addr 0x3C");
  u8g2.sendBuffer();
  delay(1000);
}
