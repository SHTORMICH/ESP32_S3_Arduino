#include <Arduino.h>

constexpr int LED_PIN = 4;
constexpr int BUTTON_PIN = 0;
constexpr unsigned long BLINK_INTERVAL = 1000;
constexpr unsigned long DEBOUNCE_MS = 50;

enum class LedState { On, Off };

class Led {
    const int pin;
    LedState state = LedState::Off;

public:
    explicit Led(int p) : pin(p) {}

    void init() {
        pinMode(pin, OUTPUT);
        set(LedState::Off);
    }

    void set(LedState s) {
        state = s;
        digitalWrite(pin, state == LedState::On ? HIGH : LOW);
    }

    LedState getState() const { return state; }
};

// ISR only sets this flag — nothing else
volatile bool buttonFlag = false;

void IRAM_ATTR onButtonPress() {
    buttonFlag = true;
}

class Button {
    const int pin;
    unsigned long lastPress = 0;

public:
    explicit Button(int p) : pin(p) {}

    void init() {
        pinMode(pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(pin), onButtonPress, FALLING);
    }

    bool wasPressed() {
        if (!buttonFlag) return false;
        buttonFlag = false;

        const unsigned long now = millis();
        if (now - lastPress < DEBOUNCE_MS) return false;
        lastPress = now;
        return true;
    }
};

class LoopProfiler {
    static constexpr uint32_t REPORT_EVERY = 1000;
    uint32_t count = 0;
    unsigned long iterStart = 0;
    unsigned long maxUs = 0;
    unsigned long sumUs = 0;

public:
    void begin() { 
      iterStart = micros(); 
    }

    void end() {
        const unsigned long dur = micros() - iterStart;
        if (dur > maxUs) maxUs = dur;
        sumUs += dur;

        if (++count >= REPORT_EVERY) {
            Serial.printf("Loop x%u | avg: %lu us | max: %lu us\n",
                          REPORT_EVERY, sumUs / REPORT_EVERY, maxUs);
            count = 0;
            maxUs = 0;
            sumUs = 0;
        }
    }
};

Led led(LED_PIN);
Button button(BUTTON_PIN);
LoopProfiler profiler;

void setup() {
    Serial.begin(115200);
    led.init();
    button.init();
}

void loop() {
    profiler.begin();

    static unsigned long lastToggle = 0;
    const unsigned long now = millis();

    if (now - lastToggle >= BLINK_INTERVAL) {
        led.set(led.getState() == LedState::On ? LedState::Off : LedState::On);
        lastToggle = now;
    }

    if (button.wasPressed()) {
        led.set(LedState::Off);
        lastToggle = now;
    }

    profiler.end();
}
