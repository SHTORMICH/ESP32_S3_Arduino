#include <Arduino.h>

constexpr int redPin = 4;
constexpr int yelPin = 6;
constexpr int grePin = 15;
constexpr int butPin = 8;

const int LONG_PRESS_MS = 1000;
const int DEBOUNCE_MS = 50;

volatile bool buttonState = false;

unsigned long pressStartTime = 0;
bool longPressHandled = false;
bool lastButtonState = false;

void IRAM_ATTR buttonISR() {
  buttonState = (digitalRead(butPin) == LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(butPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(butPin), buttonISR, CHANGE);
  Serial.println("Ready!");
}

void onShortPress() {
  Serial.println("Short Button");
}

void onLongPress() {
  Serial.println("Long button");
}

void handleButton() {
  unsigned long now = millis();
  bool current = buttonState;  // читаємо volatile лише раз

  // Натиснули
  if (current && !lastButtonState) {
    if (now - pressStartTime > DEBOUNCE_MS) {
      pressStartTime = now;
      longPressHandled = false;
    }
  }

  // Утримується — перевіряємо довге
  if (current && !longPressHandled) {
    if (now - pressStartTime >= LONG_PRESS_MS) {
      longPressHandled = true;
      onLongPress();
    }
  }

  // Відпустили
  if (!current && lastButtonState) {
    if (!longPressHandled && (now - pressStartTime >= DEBOUNCE_MS)) {
      onShortPress();
    }
  }

  lastButtonState = current;
}

void loop() {
  handleButton();
}