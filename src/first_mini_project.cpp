#include <Arduino.h>

constexpr int redPin = 4;
constexpr int yelPin = 6;
constexpr int grePin = 15;
constexpr int butExt = 8;
constexpr int butInt = 0;
constexpr int ldrPin = 9; 

constexpr unsigned long ledDelay = 1000;
volatile bool internalButtonPressed = false;
volatile bool externalButtonPressed = false;

unsigned long lastLedTime = 0;
int mode = 0;          // 0 = automatic cycle, 1 = red until short press
int trafficStep = 0;   // 0=red, 1=yellow, 2=green, 3=yellow

constexpr int LDR_MIN = 50;    // ADC value in total darkness
constexpr int LDR_MAX = 1000;  // ADC value in bright light
constexpr int PWM_MIN = 30;    // minimum duty cycle — LED never turns off

bool sequenceRunning = false;
int duty = PWM_MIN;        // current brightness, updated each loop from LDR

void ARDUINO_ISR_ATTR handleIntButtonISR() {
  internalButtonPressed = true;
}

void ARDUINO_ISR_ATTR handleExtButtonISR() {
  externalButtonPressed = true;
}

void setLights(bool red, bool yellow, bool green) {
  analogWrite(redPin, red    ? duty : 0);
  analogWrite(yelPin, yellow ? duty : 0);
  analogWrite(grePin, green  ? duty : 0);
}

void change_color_auto() {
  if (millis() - lastLedTime < ledDelay) {
    return;
  }
  lastLedTime = millis();
  switch (trafficStep) {
    case 0:
      setLights(HIGH, LOW, LOW);
      Serial.println("RED");
      trafficStep = 1;
      break;
    case 1:
      setLights(LOW, HIGH, LOW);
      Serial.println("YELLOW");
      trafficStep = 2;
      break;
    case 2:
      setLights(LOW, LOW, HIGH);
      Serial.println("GREEN");
      trafficStep = 3;
      break;
    case 3:
      setLights(LOW, HIGH, LOW);
      Serial.println("YELLOW");
      trafficStep = 0;
      break;
  }
}

void change_color_manual() {
  if (!sequenceRunning) {
    setLights(HIGH, LOW, LOW);
    return;
  }
  if (millis() - lastLedTime < ledDelay) {
    return;
  }
  lastLedTime = millis();
  switch (trafficStep) {
    case 0:
      setLights(HIGH, LOW, LOW);
      Serial.println("MANUAL: RED");
      trafficStep = 1;
      break;
    case 1:
      setLights(LOW, HIGH, LOW);
      Serial.println("MANUAL: YELLOW");
      trafficStep = 2;
      break;
    case 2:
      setLights(LOW, LOW, HIGH);
      Serial.println("MANUAL: GREEN");
      trafficStep = 3;
      break;
    case 3:
      setLights(HIGH, LOW, LOW);
      Serial.println("MANUAL: BACK TO RED");
      trafficStep = 0;
      sequenceRunning = false;
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(redPin, OUTPUT);
  pinMode(yelPin, OUTPUT);
  pinMode(grePin, OUTPUT);
  pinMode(butInt, INPUT_PULLUP);
  pinMode(butExt, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(butInt), handleIntButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(butExt), handleExtButtonISR, FALLING);
}

void loop() {
  int ldrRaw = analogRead(ldrPin);
  
  int ldrClamped = constrain(ldrRaw, LDR_MIN, LDR_MAX);
  duty = map(ldrRaw, LDR_MIN, LDR_MAX, PWM_MIN, 255);
  
  if (internalButtonPressed) {
    internalButtonPressed = false;
    mode = (mode + 1) % 2;
    trafficStep = 0;
    sequenceRunning = false;
    lastLedTime = millis();
    Serial.printf("mode=%d\n", mode);
    setLights(HIGH, LOW, LOW);
  }

  if (externalButtonPressed) {
    externalButtonPressed = false;
    if (mode == 1 && !sequenceRunning) {
      sequenceRunning = true;
      trafficStep = 0;
      lastLedTime = millis();
    }
  }

  if (mode == 0) {
    change_color_auto();
  } else {
    change_color_manual();
  }
}