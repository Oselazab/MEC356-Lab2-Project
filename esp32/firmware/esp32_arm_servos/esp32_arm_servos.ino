#include <Arduino.h>
#include <stdlib.h>

constexpr uint8_t pins[] = {25, 26};
int current[] = {90, 90};
int origin[] = {90, 90};
int target[] = {90, 90};
unsigned long started = 0;
unsigned long duration = 0;
char line[32];
size_t used = 0;
bool overflowed = false;

bool pwmReady = false;
bool completionPending = false;
bool outputOk = true;

void writeServo(size_t index, int angle) {
  // Map 0-180 degrees to 1000-2000 us within a 20 ms PWM period.
  const uint32_t pulseUs = 1000 + (angle * 1000 + 90) / 180;
  const bool written = ledcWrite(pins[index], (pulseUs * 65536 + 10000) / 20000);
  outputOk = outputOk && written;
}

void updatePosition() {
  const unsigned long elapsed = millis() - started;
  for (size_t i = 0; i < 2; ++i) {
    const int next = elapsed < duration
        ? origin[i] + (long)(target[i] - origin[i]) * (long)elapsed / (long)duration
        : target[i];
    if (next != current[i]) {
      current[i] = next;
      writeServo(i, next);
    }
  }
}

void applyPacket() {
  char *cursor = line;
  long values[3];
  for (size_t i = 0; i < 3; ++i) {
    char *end;
    values[i] = strtol(cursor, &end, 10);
    if (cursor == end || *end != (i == 2 ? '\0' : ',')) {
      return;
    }
    cursor = end + 1;
  }
  if (values[0] < 0 || values[0] > 180 || values[1] < 0 ||
      values[1] > 180 || values[2] < 0 || values[2] > 60000) {
    return;
  }
  updatePosition();
  for (size_t i = 0; i < 2; ++i) {
    origin[i] = current[i];
    target[i] = values[i];
  }
  duration = values[2];
  started = millis();
  completionPending = true;
  outputOk = true;
  Serial.printf("ESP32 target: GPIO25=%d deg GPIO26=%d deg duration=%lu ms\n",
                target[0], target[1], duration);
}

void setup() {
  Serial.begin(115200);
  for (size_t i = 0; i < 2; ++i) {
    if (!ledcAttach(pins[i], 50, 16)) {
      Serial.printf("ESP32 PWM attach failed on GPIO%u\n", pins[i]);
      return;
    }
    writeServo(i, current[i]);
  }
  pwmReady = outputOk;
}

void loop() {
  if (!pwmReady) return;
  while (Serial.available()) {
    const char c = Serial.read();
    if (c == '\n') {
      if (!overflowed) {
        line[used] = '\0';
        applyPacket();
      }
      used = 0;
      overflowed = false;
    } else if (c != '\r' && !overflowed) {
      if (used + 1 < sizeof(line)) {
        line[used++] = c;
      } else {
        overflowed = true;
      }
    }
  }
  updatePosition();
  // Allow the final PWM duty to latch before reporting hardware readback.
  if (completionPending && millis() - started >= duration + 25) {
    completionPending = false;
    Serial.printf("ESP32 %s: GPIO25 duty=%lu at %lu Hz; GPIO26 duty=%lu at %lu Hz\n",
                  outputOk ? "PWM applied" : "PWM write failed",
                  (unsigned long)ledcRead(pins[0]), (unsigned long)ledcReadFreq(pins[0]),
                  (unsigned long)ledcRead(pins[1]), (unsigned long)ledcReadFreq(pins[1]));
  }
}
