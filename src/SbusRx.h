#pragma once
#include <Arduino.h>

class SbusRx {
public:
  static constexpr int kChannels = 16;

  bool begin(int rxPin, bool inverted = true, HardwareSerial& serial = Serial2);

  bool read();

  bool is_failsafe() const { return failsafe_; }
  bool is_linked() const;
  bool ch17() const { return ch17_; }
  bool ch18() const { return ch18_; }

  uint16_t operator[](int index) const { return channel(index); }
  uint16_t channel(int index) const;
  uint16_t channel_us(int index) const;

  float frame_rate() const { return frameRate_; }
  uint32_t frame_count() const { return frames_; }
  uint32_t last_frame_ms() const { return lastFrameMs_; }
  bool rx_inverted() const { return inverted_; }

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

  uint32_t frames_ = 0;
  uint32_t lastFrameMs_ = 0;
  uint32_t lastSwitchMs_ = 0;
  uint32_t windowStart_ = 0;
  uint32_t windowFrames_ = 0;
  float frameRate_ = 0;
};
