#include <Arduino.h>

constexpr int ldrPin = 4;

constexpr int rawAtDark   = 0;
constexpr int rawAtBright = 1000;
constexpr int U_ref       = 3300; // mV
constexpr int ADC         = 1023;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(ldrPin, ADC_11db);
}

void loop() {
  const int raw = analogRead(ldrPin);
  const int millivolts = analogReadMilliVolts(ldrPin);
  const int level = map(raw, rawAtDark, rawAtBright, 0, 100);

  int U_cal = raw * U_ref / ADC;
  float relativeError = 100.0f * ((float)U_cal - (float)millivolts) / (float)millivolts;

  Serial.printf("LDR (GPIO%d): raw=%d | U_cal=%d | millivolts=%d | relativeError=%f \n", ldrPin, raw, U_cal, millivolts, relativeError);
  delay(100);
}