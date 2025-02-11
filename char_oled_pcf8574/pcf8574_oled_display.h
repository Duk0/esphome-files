#pragma once

#include "esphome/core/component.h"
#include "esphome/components/char_oled_base/oled_display.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace char_oled_pcf8574 {

class PCF8574OLEDDisplay : public char_oled_base::OLEDDisplay, public i2c::I2CDevice {
 public:
  void set_writer(std::function<void(PCF8574OLEDDisplay &)> &&writer) { this->writer_ = std::move(writer); }
  void setup() override;
  void dump_config() override;

 protected:
  bool is_four_bit_mode() override { return true; }
  void write_n_bits(uint8_t value, uint8_t n) override;
  void send(uint8_t value, bool rs) override;
  void wait_for_ready() override;

  void call_writer() override { this->writer_(*this); }

  std::function<void(PCF8574OLEDDisplay &)> writer_;
};

}  // namespace char_oled_pcf8574
}  // namespace esphome
