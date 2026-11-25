#include <Arduino.h>

constexpr uint8_t servoPins[] = {25, 26};
constexpr uint32_t frequency = 50;
constexpr uint8_t resolution = 16;

bool reportAndSet(uint8_t pin, int pulseUs) {
  const uint32_t duty = (pulseUs * (1UL << resolution) + 10000) / 20000;
  const bool written = ledcWrite(pin, duty);
  // Read back after the duty has latched at a PWM period boundary.
  delay(25);
  const uint32_t actualDuty = ledcRead(pin);
  const uint32_t actualFrequency = ledcReadFreq(pin);
  Serial.printf("GPIO%u: %d us, duty=%lu, frequency=%lu Hz\n",
                pin, pulseUs, (unsigned long)actualDuty, (unsigned long)actualFrequency);
  return written && actualDuty == duty && actualFrequency == frequency;
}

void runDemo() {
  for (uint8_t pin : servoPins) {
    if (!ledcAttach(pin, frequency, resolution)) {
      Serial.printf("ERROR attaching GPIO%u\n", pin);
      for (uint8_t attached : servoPins) {
        if (attached == pin) break;
        ledcDetach(attached);
      }
      return;
    }
  }

  // Move the channels in opposite directions and finish at neutral.
  constexpr int offsetsUs[] = {0, 120, -120, 0};
  bool ok = true;
  for (int offset : offsetsUs) {
    const bool first = reportAndSet(servoPins[0], 1500 + offset);
    const bool second = reportAndSet(servoPins[1], 1500 - offset);
    if (!first || !second) {
      ok = false;
      break;
    }
    delay(1000);
  }
  for (uint8_t pin : servoPins) {
    ledcDetach(pin);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  Serial.println(ok ? "PASS PWM registers; outputs idle" : "ERROR PWM registers; outputs idle");
}

void setup() {
  Serial.begin(115200);
  for (uint8_t pin : servoPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  Serial.println("READY: send D to run the two-servo demo");
}

void loop() {
  const int command = Serial.read();
  if (command == 'D' || command == 'd') runDemo();
  if (command == '?') Serial.println("READY: send D to run the two-servo demo");
}
