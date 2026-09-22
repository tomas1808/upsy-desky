#include "feiya_display.h"

#include <cmath>
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace feiya_desk {

static const char *const TAG = "feiya_desk";

void FeiyaDisplay::setup() {
  this->invalidate_height_();
  if (bus_connected_ != nullptr) bus_connected_->publish_state(false);
  this->set_status_("Waiting for desk");
}

void FeiyaDisplay::set_status_(const char *status) {
  if (display_status_ != nullptr && (!display_status_->has_state() || display_status_->state != status))
    display_status_->publish_state(status);
}

void FeiyaDisplay::invalidate_height_() {
  if (!this->has_state() || !std::isnan(this->state)) this->publish_state(NAN);
}

void FeiyaDisplay::loop() {
  uint8_t byte;
  // Bound each pass so a noisy input cannot starve Wi-Fi or the native API.
  for (unsigned count = 0; count < 256 && this->available(); ++count) {
    if (!this->read_byte(&byte)) break;
    feiya_protocol::DisplayFrame frame;
    const uint32_t now = millis();
    if (!parser_.push(byte, now, frame)) continue;
    last_frame_ms_ = now;
    if (!connected_) {
      connected_ = true;
      if (bus_connected_ != nullptr) bus_connected_->publish_state(true);
    }
    const bool blank = frame.kind == feiya_protocol::DisplayKind::BLANK;
    numeric_frame_ = false;
    if (display_active_ != nullptr && (!display_active_->has_state() || display_active_->state == blank))
      display_active_->publish_state(!blank);
    if (blank) {
      display_ok_ = true;
      this->set_status_("Standby");
      // The controller transmits blank frames in standby, not a zero height.
      // Preserve a height only if measured during this boot and connection.
    } else if (frame.kind == feiya_protocol::DisplayKind::HEIGHT &&
               frame.height_cm >= 20.0f && frame.height_cm <= 200.0f) {
      display_ok_ = true;
      numeric_frame_ = true;
      last_height_ms_ = now;
      this->set_status_("Height");
      if (!this->has_state() || std::isnan(this->state) || std::fabs(this->state - frame.height_cm) > 0.001f) {
        this->publish_state(frame.height_cm);
        ESP_LOGI(TAG, "Height %.1f cm", frame.height_cm);
      }
    } else {
      display_ok_ = false;
      this->invalidate_height_();
      this->set_status_("Non-height display");
    }
  }
  if (connected_ && static_cast<uint32_t>(millis() - last_frame_ms_) > 1000) {
    connected_ = false;
    if (bus_connected_ != nullptr) bus_connected_->publish_state(false);
    this->invalidate_height_();
    this->set_status_("No desk data");
  }
}

void FeiyaDisplay::dump_config() {
  LOG_SENSOR("", "FEIYA desk height", this);
  ESP_LOGCONFIG(TAG, "Receive-only 5A segment decoder; checksum required");
  this->check_uart_settings(9600);
}

}  // namespace feiya_desk
}  // namespace esphome
