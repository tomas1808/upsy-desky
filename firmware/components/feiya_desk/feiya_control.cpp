#include "feiya_control.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace feiya_desk {

static const char *const TAG = "feiya_control";

void FeiyaControl::setup() {
  ready_ = true;
  this->release();
}

void FeiyaControl::send_(uint8_t button) {
  const auto bytes = feiya_protocol::command(button);
  this->write_array(bytes.data(), bytes.size());
  last_sent_ms_ = millis();
  if (button != previous_button_) {
    ESP_LOGI(TAG, "Buttons 0x%02X%s", button, button == 0 ? " (released)" : " (panel command)");
    previous_button_ = button;
  }
}

bool FeiyaControl::recall_preset(uint8_t preset) {
  const uint32_t now = millis();
  // M can put the panel into a nonnumeric save display. A following preset
  // must still be accepted while valid controller traffic is present.
  if (!ready_ || display_ == nullptr || !display_->has_recent_data(now)) {
    ESP_LOGW(TAG, "Preset rejected: no recent controller data");
    return false;
  }
  const bool accepted = control_.start(preset, now);
  ESP_LOGI(TAG, "Preset %u: %s", preset, accepted ? "button press queued" : "rejected");
  return accepted;
}

void FeiyaControl::release() {
  control_.release();
  if (ready_) this->send_(0);
}

bool FeiyaControl::save_preset(uint8_t preset) {
  const uint32_t now = millis();
  if (!ready_ || display_ == nullptr || !display_->allows_manual_motion(now)) {
    ESP_LOGW(TAG, "Save rejected: no recent valid display/standby data");
    return false;
  }
  const bool accepted = control_.save(preset, now);
  ESP_LOGI(TAG, "Save current position to preset %u: %s",
           preset, accepted ? "queued" : "rejected");
  return accepted;
}

bool FeiyaControl::jog(uint8_t button, uint32_t duration_ms) {
  const uint32_t now = millis();
  if (!ready_ || display_ == nullptr || !display_->allows_manual_motion(now)) return false;
  const bool accepted = control_.jog(button, now, duration_ms);
  ESP_LOGI(TAG, "Manual %u ms jog 0x%02X: %s", static_cast<unsigned>(duration_ms),
           button, accepted ? "queued" : "rejected");
  return accepted;
}

void FeiyaControl::cancel_manual() {
  if (control_.cancel_manual() && ready_) this->send_(0);
}

void FeiyaControl::loop() {
  const uint32_t now = millis();
  // A save-mode display may be nonnumeric. Require valid traffic, not a numeric
  // height, during the sequence. Loss of traffic cancels every queued step.
  if (display_ == nullptr || !display_->has_recent_data(now)) control_.release();
  else if (control_.is_jog() && !display_->allows_manual_motion(now)) control_.release();
  const uint8_t key = control_.key(now);
  if (static_cast<uint32_t>(now - last_sent_ms_) >= 10) this->send_(key);
}

void FeiyaControl::dump_config() {
  ESP_LOGCONFIG(TAG, "FEIYA preset recall: original panel MUST be physically disconnected");
  ESP_LOGCONFIG(TAG, "Presets 1-4. Recall: 200 ms. Save: M 1000 ms, release 300 ms, preset 200 ms");
  ESP_LOGCONFIG(TAG, "Up/Down: 500 or 5000 ms manual jog; no height-target movement");
  this->check_uart_settings(9600);
}

}  // namespace feiya_desk
}  // namespace esphome
