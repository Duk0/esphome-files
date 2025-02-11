#include "oled_display.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace char_oled_base {

static const char *const TAG = "oled";

// First set bit determines command, bits after that are the data.
static const uint8_t LCD_DISPLAY_COMMAND_CLEAR_DISPLAY = 0x01;
static const uint8_t LCD_DISPLAY_COMMAND_RETURN_HOME = 0x02;
static const uint8_t LCD_DISPLAY_COMMAND_ENTRY_MODE_SET = 0x04;
static const uint8_t LCD_DISPLAY_COMMAND_DISPLAY_CONTROL = 0x08;
static const uint8_t LCD_DISPLAY_COMMAND_CURSOR_SHIFT = 0x10;
static const uint8_t LCD_DISPLAY_COMMAND_FUNCTION_SET = 0x20;
static const uint8_t LCD_DISPLAY_COMMAND_SET_CGRAM_ADDR = 0x40;
static const uint8_t LCD_DISPLAY_COMMAND_SET_DDRAM_ADDR = 0x80;

static const uint8_t LCD_DISPLAY_ENTRY_SHIFT_INCREMENT = 0x01;
static const uint8_t LCD_DISPLAY_ENTRY_LEFT = 0x02;

static const uint8_t LCD_DISPLAY_DISPLAY_BLINK_ON = 0x01;
static const uint8_t LCD_DISPLAY_DISPLAY_CURSOR_ON = 0x02;
static const uint8_t LCD_DISPLAY_DISPLAY_ON = 0x0C;

static const uint8_t LCD_DISPLAY_FUNCTION_8_BIT_MODE = 0x10;
static const uint8_t LCD_DISPLAY_FUNCTION_2_LINE = 0x08;
static const uint8_t LCD_DISPLAY_FUNCTION_5X10_DOTS = 0x04;

void OLEDDisplay::setup() {
  this->buffer_ = new uint8_t[this->rows_ * this->columns_];  // NOLINT
  for (uint8_t i = 0; i < this->rows_ * this->columns_; i++)
    this->buffer_[i] = ' ';
/*
  uint8_t display_function = 0;

  if (!this->is_four_bit_mode())
    display_function |= LCD_DISPLAY_FUNCTION_8_BIT_MODE;

  if (this->rows_ > 1)
    display_function |= LCD_DISPLAY_FUNCTION_2_LINE;
*/
  // TODO dotsize

  // Commands can only be sent 40ms after boot-up, so let's wait if we're close
  const uint8_t now = millis();
  if (now < 40)
    delay(40u - now);
  // Initialization sequence is not quite as documented by Winstar.
  // Documented sequence only works on initial power-up.  
  // An additional step of putting back into 8-bit mode first is 
  // required to handle a warm-restart.
  //
  // In the data sheet, the timing specs are all zeros(!).  These have been tested to 
  // reliably handle both warm & cold starts.
  //
  //  next sequence is added by LZ4TU and it is described in WS0010 datasheet. 
  //  There is no problem during hot restart anymore

  delayMicroseconds(500000); // give it some time to power up
  this->write_n_bits(0x00, 4);  // my addon 
  delayMicroseconds(50000);   // my addon
  this->write_n_bits(0x00, 4);  // my addon
  delayMicroseconds(50000);   // my addon
  this->write_n_bits(0x0, 4);  // my addon
  delayMicroseconds(50000);   // my addon
  this->write_n_bits(0x00, 4);  // my addon
  delayMicroseconds(50000);   // my addon
  this->write_n_bits(0x00, 4);  // my addon
  delayMicroseconds(50000);   // my addon

  // next lines under comments by LZ4TU, because this way is not described in WS0010 datasheet
  // 4-Bit initialization sequence from Technobly

/*  delayMicroseconds(50000); // give it some time to power up

  this->write_n_bits(0x03, 4); // Put back into 8-bit mode
  delayMicroseconds(5000);
  if (this->version_ == OLED_HW_V2) {  // only run extra command for newer displays
    this->write_n_bits(0x08, 4);
    delayMicroseconds(5000);
  }*/

  if (this->is_four_bit_mode()) {
    this->write_n_bits(0x02, 4); // Put into 4-bit mode
    delayMicroseconds(5000);
    this->write_n_bits(0x02, 4);
    delayMicroseconds(5000);
    this->write_n_bits(0x08, 4); // JAPANESE
    //this->write_n_bits(0x08 | 0x01, 4); // EUROPEAN_I
    //this->write_n_bits(0x08 | 0x03, 4); // EUROPEAN_II
    //this->write_n_bits(0x08 | this->character_set_, 4);
    delayMicroseconds(5000);
  }/* else {
    this->command_(LCD_DISPLAY_COMMAND_FUNCTION_SET | display_function);
    delay(5);  // 4.1ms
    this->command_(LCD_DISPLAY_COMMAND_FUNCTION_SET | display_function);
    delayMicroseconds(150);
    this->command_(LCD_DISPLAY_COMMAND_FUNCTION_SET | display_function);
  }*/

  this->command_(0x08); // Turn Off
  delayMicroseconds(5000);
  this->command_(LCD_DISPLAY_COMMAND_CLEAR_DISPLAY); // Clear Display
  delayMicroseconds(5000);
  this->command_(0x06); // Set Entry Mode
  delayMicroseconds(5000);
  this->command_(LCD_DISPLAY_COMMAND_RETURN_HOME); // Home Cursor
  delayMicroseconds(5000);
  this->command_(LCD_DISPLAY_DISPLAY_ON);  // Turn On - enable cursor & blink
  delayMicroseconds(5000);

  // store user defined characters
  for (auto &user_defined_char : this->user_defined_chars_) {
    this->command_(LCD_DISPLAY_COMMAND_SET_CGRAM_ADDR | (user_defined_char.first << 3));
    for (auto data : user_defined_char.second)
      this->send(data, true);
  }
}

float OLEDDisplay::get_setup_priority() const { return setup_priority::PROCESSOR; }

void HOT OLEDDisplay::display() {
  this->command_(LCD_DISPLAY_COMMAND_SET_DDRAM_ADDR | 0);

  for (uint8_t i = 0; i < this->columns_; i++) {
    this->send(this->buffer_[i], true);
    this->wait_for_ready();
  }

  if (this->rows_ >= 3) {
    for (uint8_t i = 0; i < this->columns_; i++) {
      this->send(this->buffer_[this->columns_ * 2 + i], true);
      this->wait_for_ready();
    }
  }

  if (this->rows_ >= 1) {
    this->command_(LCD_DISPLAY_COMMAND_SET_DDRAM_ADDR | 0x40);

    for (uint8_t i = 0; i < this->columns_; i++) {
      this->send(this->buffer_[this->columns_ + i], true);
      this->wait_for_ready();
    }

    if (this->rows_ >= 4) {
      for (uint8_t i = 0; i < this->columns_; i++) {
        this->send(this->buffer_[this->columns_ * 3 + i], true);
        this->wait_for_ready();
      }
    }
  }
}

void OLEDDisplay::update() {
  this->clear();
  this->call_writer();
  this->display();
}

void OLEDDisplay::command_(uint8_t value) { 
  this->send(value, false);
  this->wait_for_ready();
}

void OLEDDisplay::print(uint8_t column, uint8_t row, const char *str) {
  uint8_t pos = column + row * this->columns_;
  for (; *str != '\0'; str++) {
    if (*str == '\n') {
      pos = ((pos / this->columns_) + 1) * this->columns_;
      continue;
    }
    if (pos >= this->rows_ * this->columns_) {
      ESP_LOGW(TAG, "OLEDDisplay writing out of range!");
      break;
    }

    this->buffer_[pos] = *reinterpret_cast<const uint8_t *>(str);
    pos++;
  }
}

void OLEDDisplay::print(uint8_t column, uint8_t row, const std::string &str) { this->print(column, row, str.c_str()); }
void OLEDDisplay::print(const char *str) { this->print(0, 0, str); }
void OLEDDisplay::print(const std::string &str) { this->print(0, 0, str.c_str()); }
void OLEDDisplay::printf(uint8_t column, uint8_t row, const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[256];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    this->print(column, row, buffer);
}
void OLEDDisplay::printf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  char buffer[256];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    this->print(0, 0, buffer);
}
void OLEDDisplay::clear() {
  for (uint8_t i = 0; i < this->rows_ * this->columns_; i++)
    this->buffer_[i] = ' ';
}
void OLEDDisplay::strftime(uint8_t column, uint8_t row, const char *format, ESPTime time) {
  char buffer[64];
  size_t ret = time.strftime(buffer, sizeof(buffer), format);
  if (ret > 0)
    this->print(column, row, buffer);
}
void OLEDDisplay::strftime(const char *format, ESPTime time) { this->strftime(0, 0, format, time); }
void OLEDDisplay::loadchar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7;  // we only have 8 locations 0-7
  this->command_(LCD_DISPLAY_COMMAND_SET_CGRAM_ADDR | (location << 3));
  for (int i = 0; i < 8; i++) {
    this->send(charmap[i], true);
  }
}
/*
void OLEDDisplay::set_hw_version(OLEDHardware version) {
  this->version_ = version;
}
void OLEDDisplay::set_character_set(OLEDCharacterSet character_set) {
  this->character_set_ = character_set;
}
*/
}  // namespace char_oled_base
}  // namespace esphome
