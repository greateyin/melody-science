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
#define BTN_TARE 8

// Rotary encoder (HW-040)
#define ENC_CLK 2
#define ENC_DT 3
#define ENC_SW 4

// Heating (MOSFET control)
#define HEAT_PWM 9
float targetTemp = 20.0;
float tempHys = 0.5;
float CAL_FACTOR = 903.2; // calibrated with 50g

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);
HX711 scale;

// Encoder state (interrupt-driven)
volatile int8_t enc_delta = 0;
volatile uint8_t enc_state = 0;
const int8_t enc_table[16] = {0, -1, 1, 0,
                              1, 0, 0, -1,
                              -1, 0, 0, 1,
                              0, 1, -1, 0};

void encoderISR() {
  enc_state = (enc_state << 2) | (digitalRead(ENC_CLK) << 1) | digitalRead(ENC_DT);
  enc_delta += enc_table[enc_state & 0x0F];
}

void padLine(char *line) {
  size_t len = strlen(line);
  for (size_t i = len; i < 16; i++) {
    line[i] = ' ';
  }
  line[16] = '\0';
}

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
  tempSensors.setWaitForConversion(false);
  Serial.println("DS18B20 begin");

  scale.begin(HX_DT, HX_SCK);
  scale.set_scale(CAL_FACTOR);
  pinMode(BTN_TARE, INPUT_PULLUP);
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(HEAT_PWM, OUTPUT);
  analogWrite(HEAT_PWM, 0);
  enc_state = (digitalRead(ENC_CLK) << 1) | digitalRead(ENC_DT);
  attachInterrupt(digitalPinToInterrupt(ENC_CLK), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DT), encoderISR, CHANGE);
  Serial.println("HX711 ready check pending");
  Serial.println("HX711 begin");
}

void loop() {
  static bool lastStable = HIGH;
  static bool lastRead = HIGH;
  static unsigned long lastChangeMs = 0;
  const unsigned long debounceMs = 40;

  bool btn = digitalRead(BTN_TARE);
  if (btn != lastRead) {
    lastChangeMs = millis();
    lastRead = btn;
  }

  if ((millis() - lastChangeMs) > debounceMs && btn != lastStable) {
    lastStable = btn;
    if (lastStable == LOW) {
      if (scale.is_ready()) {
        scale.tare();
        Serial.println("HX711 tare OK");
      } else {
        Serial.println("HX711 not ready, skip tare");
      }
    }
  }

  static unsigned long lastTempReqMs = 0;
  static bool tempPending = false;
  static float tempC = -1000.0;
  static bool tempValid = false;

  unsigned long now = millis();
  if (!tempPending && now - lastTempReqMs >= 1000) {
    tempSensors.requestTemperatures();
    lastTempReqMs = now;
    tempPending = true;
  }
  if (tempPending && now - lastTempReqMs >= 750) {
    tempC = tempSensors.getTempCByIndex(0);
    tempValid = (tempC > -100.0 && tempC < 150.0);
    tempPending = false;
  }

  // Encoder handling (adjust targetTemp in 1C steps)
  int8_t delta = 0;
  noInterrupts();
  if (enc_delta != 0) {
    delta = enc_delta;
    enc_delta = 0;
  }
  interrupts();
  if (delta != 0) {
    static int acc = 0;
    acc += delta;
    while (acc >= 2) {
      targetTemp += 1.0;
      acc -= 2;
    }
    while (acc <= -2) {
      targetTemp -= 1.0;
      acc += 2;
    }
    if (targetTemp < 20.0) targetTemp = 20.0;
    if (targetTemp > 100.0) targetTemp = 100.0;
  }

  // Heating control with hysteresis + time-proportional control (<= 40%) + soft-start
  static float duty = 0.0f;
  static float dutyRamp = 0.0f;
  const float dutyMax = 0.4f;
  const unsigned long heatPeriodMs = 1000;

  if (tempValid) {
    if (tempC < targetTemp - 5.0f) {
      duty = dutyMax;
    } else if (tempC < targetTemp - tempHys) {
      duty = 0.35f;
    } else if (tempC > targetTemp + tempHys) {
      duty = 0.0f;
    }
  } else {
    duty = 0.0f;
  }

  if (duty > dutyRamp) {
    dutyRamp = min(duty, dutyRamp + 0.02f);
  } else {
    dutyRamp = duty;
  }

  if (dutyRamp > dutyMax) dutyRamp = dutyMax;
  unsigned long phase = millis() % heatPeriodMs;
  unsigned long onTime = (unsigned long)(heatPeriodMs * dutyRamp);
  digitalWrite(HEAT_PWM, (phase < onTime) ? HIGH : LOW);

  bool hxReady = scale.is_ready();
  float weight_g = 0.0;
  if (hxReady) {
    weight_g = scale.get_units(10);
  }

  char line1[17];
  char line2[17];
  char tempBuf[8];
  char weightBuf[8];
  if (tempValid) {
    dtostrf(tempC, 4, 1, tempBuf);
    snprintf(line1, sizeof(line1), "T:%sC %3dC", tempBuf, (int)targetTemp);
  } else {
    snprintf(line1, sizeof(line1), "T:--.-C %3dC", (int)targetTemp);
  }
  if (hxReady) {
    float force_N = weight_g * 0.00981f;
    dtostrf(weight_g, 5, 1, weightBuf);
    char forceBuf[8];
    dtostrf(force_N, 4, 2, forceBuf);
    snprintf(line2, sizeof(line2), "W:%sg F:%sN", weightBuf, forceBuf);
  } else {
    snprintf(line2, sizeof(line2), "W:--.-- F:--.--");
  }
  padLine(line1);
  padLine(line2);

  static char lastLine1[17] = "";
  static char lastLine2[17] = "";
  bool lcdChanged = (strcmp(line1, lastLine1) != 0) || (strcmp(line2, lastLine2) != 0);
  if (lcdChanged) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    strncpy(lastLine1, line1, sizeof(lastLine1));
    strncpy(lastLine2, line2, sizeof(lastLine2));

    Serial.print("tempC=");
    Serial.print(tempC, 2);
    Serial.print(" valid=");
    Serial.print(tempValid ? "Y" : "N");
    Serial.print(" hxReady=");
    Serial.print(hxReady ? "Y" : "N");
    Serial.print(" target=");
    Serial.println(targetTemp, 1);
  }

  delay(100);
}
