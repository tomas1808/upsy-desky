#pragma once

#include <cstdint>

namespace feiya_protocol {

// Emulate a short panel button press, then return to the observed idle stream.
// Expiry releases the button; it does NOT assert that motor travel has stopped.
class PresetControl {
 public:
  static constexpr uint32_t PRESS_MS = 200;
  static constexpr uint32_t SET_HOLD_MS = 1000;
  static constexpr uint32_t SET_GAP_MS = 300;
  static constexpr uint32_t JOG_MS = 500;
  static constexpr uint32_t LONG_JOG_MS = 5000;
  enum class Phase { IDLE, RECALL, SET_HOLD, SET_GAP, SAVE_SLOT, JOG };

  bool start(uint8_t preset, uint32_t now) {
    if (preset < 1 || preset > 4) return false;
    return begin_(static_cast<uint8_t>(1U << preset), now, Phase::RECALL);
  }
  bool save(uint8_t preset, uint32_t now) {
    if (preset < 1 || preset > 4) return false;
    return begin_(static_cast<uint8_t>(1U << preset), now, Phase::SET_HOLD);
  }
  bool jog(uint8_t button, uint32_t now, uint32_t duration = JOG_MS) {
    if ((button != 0x20 && button != 0x40) ||
        (duration != JOG_MS && duration != LONG_JOG_MS)) return false;
    if (!begin_(button, now, Phase::JOG)) return false;
    jog_duration_ = duration;
    return true;
  }
  bool is_jog() const { return phase_ == Phase::JOG; }
  bool cancel_manual() {
    if (!is_jog()) return false;
    release();
    return true;
  }
  uint8_t key(uint32_t now) {
    const uint32_t elapsed = now - started_;
    switch (phase_) {
      case Phase::IDLE:
        return 0;
      case Phase::RECALL:
      case Phase::SAVE_SLOT:
        if (elapsed >= PRESS_MS) { release(); return 0; }
        return button_;
      case Phase::JOG:
        if (elapsed >= jog_duration_) { release(); return 0; }
        return button_;
      case Phase::SET_HOLD:
        if (elapsed < SET_HOLD_MS) return 0x01;
        // Do not continue a delayed sequence after the controller's save window
        // may have expired: a preset packet could then recall instead of save.
        if (elapsed > SET_HOLD_MS + 500) { release(); return 0; }
        phase_ = Phase::SET_GAP;
        started_ = now;
        return 0;
      case Phase::SET_GAP:
        if (elapsed < SET_GAP_MS) return 0;
        if (elapsed > SET_GAP_MS + 500) { release(); return 0; }
        phase_ = Phase::SAVE_SLOT;
        started_ = now;
        return button_;
    }
    return 0;
  }
  void release() { phase_ = Phase::IDLE; }

 private:
  bool begin_(uint8_t button, uint32_t now, Phase phase) {
    if (phase_ != Phase::IDLE) return false;
    button_ = button;
    phase_ = phase;
    started_ = now;
    return true;
  }
  Phase phase_{Phase::IDLE};
  uint8_t button_{0};
  uint32_t started_{0};
  uint32_t jog_duration_{JOG_MS};
};

}  // namespace feiya_protocol
