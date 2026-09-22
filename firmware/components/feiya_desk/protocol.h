#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace feiya_protocol {

enum class DisplayKind { HEIGHT, BLANK, OTHER };

struct DisplayFrame {
  DisplayKind kind{DisplayKind::OTHER};
  float height_cm{0};
};

inline int digit(uint8_t segments) {
  constexpr uint8_t patterns[]{0x3F, 0x06, 0x5B, 0x4F, 0x66,
                               0x6D, 0x7D, 0x07, 0x7F, 0x6F};
  for (int i = 0; i < 10; ++i)
    if (segments == patterns[i]) return i;
  return -1;
}

// Observed: 5A digit1 digit2 digit3 sum(digit1..3) modulo 256.
// The high bit on a digit is its decimal point. 5A 00 00 00 00 is standby.
inline DisplayFrame decode_display(const std::array<uint8_t, 5> &frame) {
  DisplayFrame result;
  if (frame[1] == 0 && frame[2] == 0 && frame[3] == 0) {
    result.kind = DisplayKind::BLANK;
    return result;
  }
  int value = 0;
  int decimal_index = -1;
  for (int i = 0; i < 3; ++i) {
    const uint8_t raw = frame[i + 1];
    int n = digit(raw & 0x7F);
    if (i == 0 && raw == 0) n = 0;  // Only a leading blank may stand for zero.
    if (n < 0) return result;
    if ((raw & 0x80) != 0) {
      if (decimal_index >= 0) return result;
      decimal_index = i;
    }
    value = value * 10 + n;
  }
  result.kind = DisplayKind::HEIGHT;
  result.height_cm = static_cast<float>(value);
  if (decimal_index == 0) result.height_cm /= 100.0f;
  if (decimal_index == 1) result.height_cm /= 10.0f;
  return result;
}

class DisplayParser {
 public:
  bool push(uint8_t byte, uint32_t now_ms, DisplayFrame &result) {
    if (size_ != 0 && static_cast<uint32_t>(now_ms - last_byte_ms_) > 100) size_ = 0;
    last_byte_ms_ = now_ms;
    if (size_ == 0 && byte != 0x5A) return false;
    bytes_[size_++] = byte;
    if (size_ < bytes_.size()) return false;
    if (static_cast<uint8_t>(bytes_[1] + bytes_[2] + bytes_[3]) == bytes_[4]) {
      result = decode_display(bytes_);
      size_ = 0;
      return true;
    }
    // Retain a possible next header after noise, missing bytes or bad checksum.
    std::size_t next = 1;
    while (next < size_ && bytes_[next] != 0x5A) ++next;
    for (std::size_t i = next; i < size_; ++i) bytes_[i - next] = bytes_[i];
    size_ -= next;
    return false;
  }

 private:
  std::array<uint8_t, 5> bytes_{};
  std::size_t size_{0};
  uint32_t last_byte_ms_{0};
};

inline std::array<uint8_t, 5> command(uint8_t buttons) {
  return {0xA5, 0x00, buttons, static_cast<uint8_t>(~buttons), 0xFF};
}

// A command cannot be extended by repeated requests or restored after reboot.
class ManualControl {
 public:
  static constexpr uint32_t DURATION_MS = 500;
  bool start(uint8_t button, uint32_t now_ms) {
    if ((button != 0x20 && button != 0x40) || key(now_ms) != 0) return false;
    button_ = button;
    started_ms_ = now_ms;
    return true;
  }
  uint8_t key(uint32_t now_ms) {
    if (static_cast<uint32_t>(now_ms - started_ms_) >= DURATION_MS) button_ = 0;
    return button_;
  }
  void cancel() { button_ = 0; }

 private:
  uint8_t button_{0};
  uint32_t started_ms_{0};
};

}  // namespace feiya_protocol
