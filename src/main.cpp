#include <Arduino.h>
#include "sbus_rx.h"

SbusRx sbus;

void setup() {
  Serial0.begin(115200);
  sbus.begin(2);
}

void loop() {
  sbus.update();

  static uint32_t lastPrint = 0;
  uint32_t now = millis();
  if (now - lastPrint < 250) return;
  lastPrint = now;

  if (!sbus.isLinked()) {
    Serial0.printf("[SBUS] no link (frames=%lu fs=%d)  CH:",
                   (unsigned long)sbus.frameCount(), sbus.isFailsafe());
    for (int i = 0; i < SbusRx::kChannels; i++) {
      Serial0.printf(" %d:%u", i + 1, sbus[i]);
    }
    Serial0.println();
    return;
  }

  Serial0.printf("[SBUS] %.1f fr/s  fs=%d  CH:", sbus.frameRate(), sbus.isFailsafe());  for (int i = 0; i < SbusRx::kChannels; i++) {
    Serial0.printf(" %d:%u", i + 1, sbus[i]);
  }
  Serial0.println();
}
