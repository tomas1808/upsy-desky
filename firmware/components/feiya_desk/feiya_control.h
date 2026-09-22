#pragma once

#include "feiya_display.h"
#include "preset_control.h"

namespace esphome {
namespace feiya_desk {

// ONLY for a physically disconnected handset. The board's RJ45 nets are shared.
class FeiyaControl : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void on_shutdown() override { this->release(); }
  void set_display(FeiyaDisplay *display) { display_ = display; }
  bool recall_preset(uint8_t preset);
  bool save_preset(uint8_t preset);
  bool jog(uint8_t button, uint32_t duration_ms = 500);
  void cancel_manual();
  void release();

 protected:
  void send_(uint8_t button);
  FeiyaDisplay *display_{nullptr};
  feiya_protocol::PresetControl control_;
  uint32_t last_sent_ms_{0};
  uint8_t previous_button_{0};
  bool ready_{false};
};

}  // namespace feiya_desk
}  // namespace esphome
