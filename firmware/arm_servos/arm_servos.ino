#include <Servo.h>
#include <stdlib.h>

constexpr uint8_t pins[] = {9, 10};
Servo servos[2];
int current[] = {90, 90};
int origin[] = {90, 90};
int target[] = {90, 90};
unsigned long started = 0;
unsigned long duration = 0;
char line[32];
size_t used = 0;
bool overflowed = false;

void updatePosition() {
  const unsigned long elapsed = millis() - started;
  for (size_t i = 0; i < 2; ++i) {
    const int next = elapsed < duration
        ? origin[i] + (long)(target[i] - origin[i]) * (long)elapsed / (long)duration
        : target[i];
    if (next != current[i]) {
      current[i] = next;
      servos[i].write(next);
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
}

void setup() {
  Serial.begin(115200);
  for (size_t i = 0; i < 2; ++i) {
    servos[i].attach(pins[i]);
    servos[i].write(current[i]);
  }
}

void loop() {
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
}
