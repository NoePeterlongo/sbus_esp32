#pragma once
#include <Arduino.h>

class SbusRx {
public:
  static constexpr int kChannels = 16;

  bool begin(int rxPin, bool inverted = true, HardwareSerial& serial = Serial2);
  void update();

  bool isNewFrame() const { return newFrame_; }
  bool isFailsafe() const { return failsafe_; }
  bool isLinked() const;
  bool ch17() const { return ch17_; }
  bool ch18() const { return ch18_; }

  uint16_t channel(int index) const;
  uint16_t operator[](int index) const { return channel(index); }
  float frameRate() const { return frameRate_; }
  uint32_t frameCount() const { return frames_; }

private:
  void restart();
  void handleFrame();

  HardwareSerial* serial_ = nullptr;
  int rxPin_ = -1;
  bool inverted_ = true;

  uint8_t buf_[25];
  size_t idx_ = 0;
  uint32_t lastByteMs_ = 0;

  uint16_t channels_[kChannels] = {};
  bool failsafe_ = false;
  bool ch17_ = false;
  bool ch18_ = false;
  bool newFrame_ = false;

  uint32_t frames_ = 0;
  uint32_t lastFrameMs_ = 0;
  uint32_t lastSwitchMs_ = 0;
  uint32_t windowStart_ = 0;
  uint32_t windowFrames_ = 0;
  float frameRate_ = 0;
};
