#include <Arduino.h>
#include <SbusRx.h>

SbusRx sbus;

void setup() {
  Serial0.begin(115200);
  sbus.begin(2);
}

void loop() {
  if (sbus.read()) {
    Serial0.printf("[SBUS] %.1f fr/s  fs=%d  link=%d  CH:",
                   sbus.frame_rate(), sbus.is_failsafe(), sbus.is_linked());
    for (int i = 0; i < SbusRx::kChannels; i++) {
      Serial0.printf(" %d:%u", i + 1, sbus[i]);
    }
    Serial0.println();
  }
}
