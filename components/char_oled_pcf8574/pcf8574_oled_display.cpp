#include "pcf8574_oled_display.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace char_oled_pcf8574 {

static const char *const TAG = "char_oled_pcf8574";


void PCF8574OLEDDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PCF8574 OLED Display...");
/*  this->backlight_value_ = LCD_DISPLAY_BACKLIGHT_ON;
  if (!this->write_bytes(this->backlight_value_, nullptr, 0)) {
    this->mark_failed();
    return;
  }*/
  this->write_bytes(0x00, nullptr, 0);

  OLEDDisplay::setup();
}
void PCF8574OLEDDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "PCF8574 OLED Display:");
  ESP_LOGCONFIG(TAG, "  Columns: %u, Rows: %u", this->columns_, this->rows_);
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with OLED Display failed!");
  }
}
void PCF8574OLEDDisplay::write_n_bits(uint8_t value, uint8_t n) {
  if (n == 4) {
    // Ugly fix: in the super setup() with n == 4 value needs to be shifted left
    value <<= 4;
  }
  //uint8_t data = value | this->backlight_value_;  // Set backlight state
  //uint8_t data[2];
  //data[0] = value;
  //data[1] = value >> 8;
  this->write_bytes(value, nullptr, 0);
  //this->write(data, 1);
  // Pulse ENABLE
  delayMicroseconds(50);
  this->write_bytes(value | 0x04, nullptr, 0);
  //data[0] = value | 0x04;
  //this->write(data, 1);
  delayMicroseconds(50);
  this->write_bytes(value, nullptr, 0);
  //data[0] = value;
  //this->write(data, 1);
}
void PCF8574OLEDDisplay::send(uint8_t value, bool rs) {
  this->write_n_bits((value & 0xF0) | rs, 0); // 0x02 = rw
  this->write_n_bits(((value << 4) & 0xF0) | rs, 0); // 0x02 = rw
}
// Poll the busy bit until it goes LOW
void PCF8574OLEDDisplay::wait_for_ready() {
/*
  //unsigned char busy = 1;
  bool busy = true;
  uint8_t data;
  uint8_t attempts = 10;
  //uint8_t busy = 1;
  //this->_busy_pin->setup();  // INPUT
  //this->_busy_pin->pin_mode(gpio::FLAG_INPUT);
  //this->rs_pin_->digital_write(false);
  //this->rw_pin_->digital_write(true);
  //this->write_bytes(0x00, nullptr, 0);
  this->write_bytes(0x02, nullptr, 0); // 0x02 = rw
  //this->write(0x02, 1); // 0x02 = rw
 
  do
  {
    //this->enable_pin_->digital_write(false);
    //this->enable_pin_->digital_write(true);
    //this->write_bytes(0x02, nullptr, 0); // 0x02 = rw
    this->write_bytes(0x02 | 0x04 | 0x80, nullptr, 0); // 0x02 = rw, 0x04 = en, 0x80 = set read D7 - busy_pin
    //this->write(0x02 | 0x04, 1); // 0x02 = rw, 0x04 = en

    delayMicroseconds(10);
    //busy = this->_busy_pin->digital_read();
    //[09:42:11][  1094][E][Wire.cpp:513] requestFrom(): i2cRead returned Error 258
    //[09:42:11][I][esp-idf:000]: E (1165) i2c: i2c_master_read(1255): i2c data read length error
    //this->read_bytes(0, &data, 1); // D7 - busy_pin
    this->read_bytes_raw(&data, 1);
    //this->read_byte(0x80, &data);
    //this->read_register(0x80, &data, 1, true);
    busy = data & 0x80;
    //ESP_LOGI(TAG, "data: %#02x, busy: %d, attempts: %d", data, busy, attempts);
    //delayMicroseconds(1);

    //this->enable_pin_->digital_write(false);
    this->write_bytes(0x02, nullptr, 0); // 0x02 = rw
    //this->write(0x02, 1); // 0x02 = rw
  	
  	// get remaining 4 bits, which are not used.
    //this->enable_pin_->digital_write(true);
    this->write_bytes(0x02 | 0x04, nullptr, 0); // 0x02 = rw, 0x04 = en
    //this->write(0x02 | 0x04, 1); // 0x02 = rw, 0x04 = en
    delayMicroseconds(50);
    //this->enable_pin_->digital_write(false);
    this->write_bytes(0x02, nullptr, 0); // 0x02 = rw
    //this->write(0x02, 1); // 0x02 = rw
    
    attempts--;
    //ESP_LOGI(TAG, "end, attempts: %d", attempts);
  }
  while(busy && attempts > 0);
  //while(busy);
  
  //this->_busy_pin->pin_mode(gpio::FLAG_OUTPUT);
  //this->rw_pin_->digital_write(false);
  this->write_bytes(0x00, nullptr, 0);
  //this->write(0x00, 1);*/
}

}  // namespace char_oled_pcf8574
}  // namespace esphome
