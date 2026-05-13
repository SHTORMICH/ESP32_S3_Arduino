#include <Arduino.h>

constexpr int BUTTON_PIN = 4;
volatile boolean emergencyButtonPressed = false;
volatile bool buttonConfirmed = false;
volatile unsigned long lastButtonPressTime = 0;
volatile unsigned int pressCount = 0;
const unsigned long DEBOUNCE_TIME = 50;

enum ButtonState { BTN_WAIT, BTN_DEBOUNCING, BTN_ACTIVE, BTN_RELEAS };
static ButtonState btnState = BTN_WAIT;
static unsigned long btnTimer  = 0;
static unsigned long lastPoll  = 0;
constexpr unsigned long POLL_INTERVAL_MS    = 10;
constexpr unsigned long DEBOUNCE_PRESS_MS   = 50;
constexpr unsigned long DEBOUNCE_RELEASE_MS = 50;

// Завдання 1: Базова реалізація
// setup(): attach this ISR on FALLING edge
// loop():  check emergencyButtonPressed flag and print count
void IRAM_ATTR withoutDebounsButtonISR() {
  pressCount++;
  buttonConfirmed = true;
}

// Завдання 2: Software debounce через таймер (time-based)
// setup(): attach this ISR on FALLING edge
// loop():  call handleSoftwareDebounce()
void IRAM_ATTR softwareDebounsButtonISR() {
  emergencyButtonPressed = true;  // ISR only signals — no debounce logic here
}
 
// Time-based debounce handler — must be called from loop(), not from ISR
void handleSoftwareDebounce() {
  if (!emergencyButtonPressed) return;
  emergencyButtonPressed = false;

  unsigned long now = millis();
  if (now - lastButtonPressTime > DEBOUNCE_TIME) {
    lastButtonPressTime = now;
    pressCount++;
    buttonConfirmed = true;
  }
}

// Завдання 3: Debounce через перевірку рівня (state-based)
// setup(): attach this ISR on CHANGE (both edges)
// loop():  check buttonConfirmed flag, then read pin state — react only if still LOW
// ISR only signals the event — all filtering logic is in loop()
void IRAM_ATTR stateBasedDebounsButtonISR() {
  pressCount++;
  buttonConfirmed = true;
}

void handleStateBasedDebounce() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    pressCount++;
  }
}

// Завдання 4: Polling + debounce (без interrupts)
// setup(): pinMode only, no attachInterrupt
// loop():  call pollButton() — no interrupt handling needed
void pollButton() {
  unsigned long now = millis();
  if (now - lastPoll < POLL_INTERVAL_MS) return;
  lastPoll = now;

  bool isPressed = (digitalRead(BUTTON_PIN) == LOW);

  switch (btnState) {
    case BTN_WAIT:
      if (isPressed) {
        btnState = BTN_DEBOUNCING;
        btnTimer = now;
      }
      break;

    case BTN_DEBOUNCING:
      if (!isPressed) {
        btnState = BTN_WAIT;  // glitch — no reaction
      } else if (now - btnTimer >= DEBOUNCE_PRESS_MS) {
        btnState = BTN_ACTIVE;
        pressCount++;
        buttonConfirmed = true;
      }
      break;

    case BTN_ACTIVE:
      if (!isPressed) {
        btnState = BTN_RELEAS;
        btnTimer = now;
      }
      break;

    case BTN_RELEAS:
      if (isPressed) {
        btnState = BTN_ACTIVE;  // glitch on release
      } else if (now - btnTimer >= DEBOUNCE_RELEASE_MS) {
        btnState = BTN_WAIT;
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // CHANGE: catches both edges (press and release),
  // so loop() decides which event to accept
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), stateBasedDebounsButtonISR, CHANGE);
}

void loop() {
  if (buttonConfirmed) {
    buttonConfirmed = false;
    Serial.printf("Button pressed! Count: %u\n", pressCount);
  }
}