#pragma once

#include "esphome/core/component.h"
#include "esphome/core/time.h"

#include <map>
#include <vector>

namespace esphome {
namespace char_oled_base {
/*
enum OLEDHardware {
  OLED_HW_V1 = 0x01,
  OLED_HW_V2
};

enum OLEDCharacterSet {
	OLED_JAPANESE = 0x00,
	OLED_EUROPEAN_I = 0x01,
	OLED_RUSSIAN = 0x02,
	OLED_EUROPEAN_II = 0x03
};
*/
class OLEDDisplay;

class OLEDDisplay : public PollingComponent {
 public:
  void set_dimensions(uint8_t columns, uint8_t rows) {
    this->columns_ = columns;
    this->rows_ = rows;
  }

  void set_user_defined_char(uint8_t pos, const std::vector<uint8_t> &data) { this->user_defined_chars_[pos] = data; }
  //void set_hw_version(OLEDHardware version);
  //void set_character_set(OLEDCharacterSet character_set);

  void setup() override;
  float get_setup_priority() const override;
  void update() override;
  void display();
  //// Clear LCD display
  void clear();

  /// Print the given text at the specified column and row.
  void print(uint8_t column, uint8_t row, const char *str);
  /// Print the given string at the specified column and row.
  void print(uint8_t column, uint8_t row, const std::string &str);
  /// Print the given text at column=0 and row=0.
  void print(const char *str);
  /// Print the given string at column=0 and row=0.
  void print(const std::string &str);
  /// Evaluate the printf-format and print the text at the specified column and row.
  void printf(uint8_t column, uint8_t row, const char *format, ...) __attribute__((format(printf, 4, 5)));
  /// Evaluate the printf-format and print the text at column=0 and row=0.
  void printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

  /// Evaluate the strftime-format and print the text at the specified column and row.
  void strftime(uint8_t column, uint8_t row, const char *format, ESPTime time) __attribute__((format(strftime, 4, 0)));
  /// Evaluate the strftime-format and print the text at column=0 and row=0.
  void strftime(const char *format, ESPTime time) __attribute__((format(strftime, 2, 0)));

  /// Load custom char to given location
  void loadchar(uint8_t location, uint8_t charmap[]);
  
  //OLEDHardware check_hwver_ = OLED_HW_V1;

 protected:
  virtual bool is_four_bit_mode() = 0;
  virtual void write_n_bits(uint8_t value, uint8_t n) = 0;
  virtual void send(uint8_t value, bool rs) = 0;
  virtual void wait_for_ready();

  void command_(uint8_t value);
  virtual void call_writer() = 0;

  uint8_t columns_;
  uint8_t rows_;
  uint8_t *buffer_{nullptr};
  std::map<uint8_t, std::vector<uint8_t> > user_defined_chars_;
  
  //OLEDHardware version_{OLED_HW_V1};
  //uint8_t character_set_{OLED_JAPANESE};
};

}  // namespace char_oled_base
}  // namespace esphome
