#include <Arduino.h>

constexpr int  RELAY_PIN   = 15;
constexpr int  CONTACT_PIN = 4;
constexpr long INTERVAL_MS = 2000;
constexpr int  MEAS_COUNT  = 10;

volatile bool          contactChanged = false;
volatile bool          contactState   = false;
volatile unsigned long isrTime_us     = 0;

unsigned long coilOnTime_us = 0;
long          samples_us[MEAS_COUNT];
int           sampleIndex = 0;
bool          measuring   = false;

bool          relayState = false;
unsigned long lastToggle = 0;

void IRAM_ATTR onContactChange() {
  isrTime_us     = micros();
  contactState   = digitalRead(CONTACT_PIN);
  contactChanged = true;
}

void printUs(long us) {
  Serial.printf("%ld.%03ld ms  (%ld µs)", us / 1000, us % 1000, us);
}

void printStats() {
  long sum = 0;
  long mn  = samples_us[0], mx = samples_us[0];
  for (int i = 0; i < MEAS_COUNT; i++) {
    sum += samples_us[i];
    mn = min(mn, samples_us[i]);
    mx = max(mx, samples_us[i]);
  }
  long avg = sum / MEAS_COUNT;

  Serial.println("\n========== RESULTS ==========");
  Serial.printf("Samples : %d\n", MEAS_COUNT);
  Serial.print ("Average : "); printUs(avg); Serial.println();
  Serial.print ("Min     : "); printUs(mn);  Serial.println();
  Serial.print ("Max     : "); printUs(mx);  Serial.println();
  Serial.printf("Spread  : %ld µs\n", mx - mn);
  Serial.println("=============================\n");
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN,   OUTPUT);
  pinMode(CONTACT_PIN, INPUT_PULLDOWN);
  digitalWrite(RELAY_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(CONTACT_PIN), onContactChange, CHANGE);

  Serial.println("=== Relay Response Timer ===");
  Serial.printf("Control : GPIO%d\n", RELAY_PIN);
  Serial.printf("Contact : GPIO%d  (IRQ, micros)\n", CONTACT_PIN);
  Serial.printf("Interval: %ld ms  |  Need %d samples\n", INTERVAL_MS, MEAS_COUNT);
  Serial.println("============================\n");

  lastToggle = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastToggle >= INTERVAL_MS) {
    lastToggle = now;
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);

    if (relayState && sampleIndex < MEAS_COUNT) {
      coilOnTime_us = micros();
      measuring     = true;
    }

    Serial.printf("[%7lu ms] Relay coil -> %s\n",
                  now, relayState ? "ON " : "OFF");
  }

  if (contactChanged) {
    noInterrupts();
    bool          isClosed   = contactState;
    unsigned long irqTime_us = isrTime_us;
    contactChanged           = false;
    interrupts();

    Serial.printf("[%7lu ms] Contact    -> %s  (IRQ)\n",
                  irqTime_us / 1000, isClosed ? "CLOSED" : "OPEN  ");

    if (measuring && isClosed && sampleIndex < MEAS_COUNT) {
      long us = (long)(irqTime_us - coilOnTime_us);
      samples_us[sampleIndex] = us;
      measuring = false;

      Serial.printf("  [sample %2d/%d]  response = ",
                    sampleIndex + 1, MEAS_COUNT);
      printUs(us);
      Serial.println();

      sampleIndex++;
      if (sampleIndex == MEAS_COUNT) printStats();
    }
  }
}
