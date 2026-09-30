#include <Arduino.h>
#include <SbusRx.h>

#ifndef SBUS_RX_PIN
#define SBUS_RX_PIN 2
#endif

SbusRx sbus;

void setup() {
  Serial.begin(115200);
  sbus.begin(SBUS_RX_PIN);
}

void loop() {
  if (sbus.read()) {
    Serial.printf("[SBUS] %.1f fr/s  fs=%d  link=%d  CH:",
                  sbus.frame_rate(), sbus.is_failsafe(), sbus.is_linked());
    for (int i = 0; i < SbusRx::kChannels; i++) {
      Serial.printf(" %d:%u", i + 1, sbus[i]);
    }
    Serial.println();
  }
}
