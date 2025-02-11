#include "gpio_oled_display.h"
#include "esphome/core/log.h"

namespace esphome {
namespace char_oled_gpio {

static const char *const TAG = "char_oled_gpio";

void GPIOOLEDDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GPIO OLED Display...");
  this->rs_pin_->setup();  // OUTPUT
  this->rs_pin_->digital_write(false);
  this->rw_pin_->setup();  // OUTPUT
  this->rw_pin_->digital_write(false);
  this->enable_pin_->setup();  // OUTPUT
  this->enable_pin_->digital_write(false);

  for (uint8_t i = 0; i < (uint8_t) (this->is_four_bit_mode() ? 4u : 8u); i++) {
    this->data_pins_[i]->setup();  // OUTPUT
    this->data_pins_[i]->digital_write(false);
  }
  OLEDDisplay::setup();
}
void GPIOOLEDDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "GPIO OLED Display:");
  ESP_LOGCONFIG(TAG, "  Columns: %u, Rows: %u", this->columns_, this->rows_);
  //ESP_LOGCONFIG(TAG, "  Version: %s", this->version_ == this->check_hwver_ ? "HW_V1" : "HW_V2");
  //ESP_LOGCONFIG(TAG, "  CharSet: %#02x", this->character_set_);
  LOG_PIN("  RS Pin: ", this->rs_pin_);
  LOG_PIN("  RW Pin: ", this->rw_pin_);
  LOG_PIN("  Enable Pin: ", this->enable_pin_);

  LOG_PIN("  Data Pin 0: ", data_pins_[0]);
  LOG_PIN("  Data Pin 1: ", data_pins_[1]);
  LOG_PIN("  Data Pin 2: ", data_pins_[2]);
  LOG_PIN("  Data Pin 3: ", data_pins_[3]);
  if (!is_four_bit_mode()) {
    LOG_PIN("  Data Pin 4: ", data_pins_[4]);
    LOG_PIN("  Data Pin 5: ", data_pins_[5]);
    LOG_PIN("  Data Pin 6: ", data_pins_[6]);
    LOG_PIN("  Data Pin 7: ", data_pins_[7]);
  }
  LOG_UPDATE_INTERVAL(this);
}
void GPIOOLEDDisplay::write_n_bits(uint8_t value, uint8_t n) {
  for (uint8_t i = 0; i < n; i++)
    this->data_pins_[i]->digital_write((value >> i) & 0x01);

  delayMicroseconds(50);

  this->enable_pin_->digital_write(true);
  delayMicroseconds(50);
  this->enable_pin_->digital_write(false);
}
void GPIOOLEDDisplay::send(uint8_t value, bool rs) {
  this->rs_pin_->digital_write(rs);
  this->rw_pin_->digital_write(false);

  if (this->is_four_bit_mode()) {
    this->write_n_bits(value >> 4, 4);
    this->write_n_bits(value, 4);
  } else {
    this->write_n_bits(value, 8);
  }
}
// Poll the busy bit until it goes LOW
void GPIOOLEDDisplay::wait_for_ready() {
  //unsigned char busy = 1;
  bool busy = true;
  uint8_t attempts = 10;
  this->_busy_pin->setup();  // INPUT
  this->_busy_pin->pin_mode(gpio::FLAG_INPUT);
  this->rs_pin_->digital_write(false);  
  this->rw_pin_->digital_write(true);
 
  do
  {
    this->enable_pin_->digital_write(false);
    this->enable_pin_->digital_write(true);

    delayMicroseconds(10);
    busy = this->_busy_pin->digital_read();
    //busy = this->data_pins_[3]->digital_read();
    this->enable_pin_->digital_write(false);
  	
  	// get remaining 4 bits, which are not used.
    this->enable_pin_->digital_write(true);
    delayMicroseconds(50);
    this->enable_pin_->digital_write(false);
    attempts--;
  }
  while(busy && attempts > 0);
  //while(busy);
  
  this->_busy_pin->pin_mode(gpio::FLAG_OUTPUT);
  this->rw_pin_->digital_write(false);
}

}  // namespace char_oled_gpio
}  // namespace esphome
