#include <Arduino.h>

#define LED_BUT 8
#define LED_RED 9

bool blinking = false;
bool lastButtonState = HIGH;

unsigned long pressStartTime = 0;
bool longPressHandled = false;

int mode = 0;
int modesCount = 4;

void setup() {
  pinMode(LED_BUT, INPUT_PULLUP);
  pinMode(LED_RED, OUTPUT);
}

void loop() {
  bool currentState = digitalRead(LED_BUT);

  // натиснули кнопку
  if (currentState == LOW && lastButtonState == HIGH) {
    pressStartTime = millis();
    longPressHandled = false;
  }

  // утримувати кнопку
  if (currentState == LOW && !longPressHandled) {
    if (millis() - pressStartTime > 800) {
      mode++;
      if (mode >= modesCount) {
        mode = 0;
      }
      longPressHandled = true;
    }
  }

  // відпустили кнопку
  if (currentState == HIGH && lastButtonState == LOW) {
    if (!longPressHandled) {
      blinking = !blinking;
      delay(20);
    }
  }

  lastButtonState = currentState;

  if (blinking) {
    switch (mode) {
      case 0:
        // повільне миготіння
        digitalWrite(LED_RED, HIGH);
        delay(500);
        digitalWrite(LED_RED, LOW);
        delay(500);
        break;

      case 1:
        // швидке миготіння
        digitalWrite(LED_RED, HIGH);
        delay(150);
        digitalWrite(LED_RED, LOW);
        delay(150);
        break;

      case 2:
        // дуже швидке миготіння
        digitalWrite(LED_RED, HIGH);
        delay(70);
        digitalWrite(LED_RED, LOW);
        delay(70);
        break;

      case 3:
        // два коротких спалахи
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(100);
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(400);
        break;

      default:
        digitalWrite(LED_RED, LOW);
        break;
    }
  } else {
    digitalWrite(LED_RED, LOW);
  }
}