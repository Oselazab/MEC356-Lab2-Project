#include <Arduino.h>

constexpr unsigned long baudRate = 115200;
char packet[24];
size_t length = 0;
bool discard = false;

bool readNumber(const char *&cursor, uint32_t &value, uint32_t maximum) {
  if (*cursor < '0' || *cursor > '9') {
    return false;
  }
  value = 0;
  do {
    value = value * 10 + (*cursor++ - '0');
    if (value > maximum) {
      return false;
    }
  } while (*cursor >= '0' && *cursor <= '9');
  return true;
}

void reportPacket() {
  const char *cursor = packet;
  uint32_t first, second, duration;
  if (!readNumber(cursor, first, 180) || *cursor++ != ',' ||
      !readNumber(cursor, second, 180) || *cursor++ != ',' ||
      !readNumber(cursor, duration, 60000) || *cursor != '\0') {
    Serial.println("ESP32 invalid command; expected servo1,servo2,duration_ms");
    return;
  }
  Serial.printf("ESP32 servo 1: %lu deg | servo 2: %lu deg | duration: %lu ms\n",
                static_cast<unsigned long>(first), static_cast<unsigned long>(second),
                static_cast<unsigned long>(duration));
}

void setup() {
  Serial.begin(baudRate);
}

void loop() {
  while (Serial.available() > 0) {
    const int byte = Serial.read();
    if (byte == '\n') {
      if (discard) {
        Serial.println("ESP32 invalid command; malformed or too long");
      } else if (length > 0) {
        if (packet[length - 1] == '\r') {
          --length;
        }
        packet[length] = '\0';
        reportPacket();
      }
      length = 0;
      discard = false;
    } else if (!discard) {
      if (byte == 0 || length == sizeof(packet) - 1) {
        discard = true;
      } else {
        packet[length++] = static_cast<char>(byte);
      }
    }
  }
}
