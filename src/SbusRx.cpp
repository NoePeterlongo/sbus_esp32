#include "SbusRx.h"

bool SbusRx::begin(int rxPin, bool inverted, HardwareSerial& serial) {
  rxPin_ = rxPin;
  inverted_ = inverted;
  serial_ = &serial;
  restart();
  return true;
}

void SbusRx::restart() {
  serial_->end();
  serial_->setRxBufferSize(256);
  serial_->begin(100000, SERIAL_8E2, rxPin_, -1);
  serial_->setRxInvert(inverted_);
  idx_ = 0;
  frames_ = 0;
  windowFrames_ = 0;
  windowStart_ = millis();
  lastSwitchMs_ = millis();
}

void SbusRx::handleFrame() {
  static constexpr uint8_t kShifts[16] = {0, 3, 6, 1, 4, 7, 2, 5,
                                          0, 3, 6, 1, 4, 7, 2, 5};

  for (int i = 0; i < kChannels; i++) {
    size_t byteIdx = 1 + (i * 11) / 8;
    uint8_t shift = kShifts[i];
    uint32_t v = (uint32_t)buf_[byteIdx] | ((uint32_t)buf_[byteIdx + 1] << 8);
    if (shift >= 6) v |= (uint32_t)buf_[byteIdx + 2] << 16;
    channels_[i] = (v >> shift) & 0x07FF;
  }

  failsafe_ = buf_[23] & 0x04;
  ch17_ = buf_[23] & 0x01;
  ch18_ = buf_[23] & 0x02;

  frames_++;
  lastFrameMs_ = millis();

  windowFrames_++;
  uint32_t now = millis();
  if (now - windowStart_ >= 1000) {
    frameRate_ = windowFrames_ * 1000.0f / (now - windowStart_);
    windowFrames_ = 0;
    windowStart_ = now;
  }
}

bool SbusRx::read() {
  if (!serial_) return false;
  bool gotFrame = false;

  while (serial_->available()) {
    uint8_t b = serial_->read();
    if (idx_ == 0) {
      if (b == 0x0F) {
        buf_[0] = b;
        idx_ = 1;
        lastByteMs_ = millis();
      }
      continue;
    }
    if (millis() - lastByteMs_ > 10) idx_ = 0;
    if (idx_ == 0) continue;
    buf_[idx_++] = b;
    lastByteMs_ = millis();
    if (idx_ == 25) {
      handleFrame();
      idx_ = 0;
      gotFrame = true;
    }
  }

  if (frames_ == 0 && millis() - lastSwitchMs_ > 1500) {
    inverted_ = !inverted_;
    restart();
  }

  return gotFrame;
}

bool SbusRx::is_linked() const {
  return !failsafe_ && lastFrameMs_ != 0 && millis() - lastFrameMs_ < 100;
}

uint16_t SbusRx::channel(int index) const {
  return (index >= 0 && index < kChannels) ? channels_[index] : 0;
}

uint16_t SbusRx::channel_us(int index) const {
  uint32_t raw = channel(index);
  if (raw < 172) return 1000;
  if (raw > 1811) return 2000;
  return 1000 + (raw - 172) * 1000 / 1639;
}
