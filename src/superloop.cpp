#include <Arduino.h>

constexpr int RED_LED_PIN   = 4;
constexpr int BLUE_LED_PIN  = 5;
constexpr int WHITE_LED_PIN = 6;
constexpr unsigned long RED_BLINK_INTERVAL   = 200;
constexpr unsigned long BLUE_BLINK_INTERVAL  = 500;
constexpr unsigned long WHITE_BLINK_INTERVAL = 1000;

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
}

void loop() {
  static unsigned long lastToggleRed   = 0;
  static unsigned long lastToggleBlue  = 0;
  static unsigned long lastToggleWhite = 0;
  const unsigned long now = millis();

  if (now - lastToggleRed >= RED_BLINK_INTERVAL) {
    lastToggleRed = now;
    digitalWrite(RED_LED_PIN, !digitalRead(RED_LED_PIN));
  }
 
  if (now - lastToggleBlue >= BLUE_BLINK_INTERVAL) {
    lastToggleBlue = now;
    digitalWrite(BLUE_LED_PIN, !digitalRead(BLUE_LED_PIN));
  }

  if (now - lastToggleWhite >= WHITE_BLINK_INTERVAL) {
    lastToggleWhite = now;
    digitalWrite(WHITE_LED_PIN, !digitalRead(WHITE_LED_PIN));
  }
}