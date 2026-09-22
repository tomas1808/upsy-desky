#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "protocol.h"

namespace esphome {
namespace feiya_desk {

class FeiyaDisplay : public sensor::Sensor, public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_display_active(binary_sensor::BinarySensor *sensor) { display_active_ = sensor; }
  void set_bus_connected(binary_sensor::BinarySensor *sensor) { bus_connected_ = sensor; }
  void set_display_status(text_sensor::TextSensor *sensor) { display_status_ = sensor; }
  bool has_recent_data(uint32_t now_ms) const {
    return connected_ && static_cast<uint32_t>(now_ms - last_frame_ms_) <= 250;
  }
  bool allows_manual_motion(uint32_t now_ms) const {
    return connected_ && display_ok_ && static_cast<uint32_t>(now_ms - last_frame_ms_) <= 250;
  }
  bool has_fresh_height(uint32_t now_ms) const {
    return allows_manual_motion(now_ms) && numeric_frame_ &&
           static_cast<uint32_t>(now_ms - last_height_ms_) <= 250;
  }

 protected:
  void set_status_(const char *status);
  void invalidate_height_();
  feiya_protocol::DisplayParser parser_;
  binary_sensor::BinarySensor *display_active_{nullptr};
  binary_sensor::BinarySensor *bus_connected_{nullptr};
  text_sensor::TextSensor *display_status_{nullptr};
  uint32_t last_frame_ms_{0};
  uint32_t last_height_ms_{0};
  bool numeric_frame_{false};
  bool connected_{false};
  bool display_ok_{false};
};

}  // namespace feiya_desk
}  // namespace esphome
