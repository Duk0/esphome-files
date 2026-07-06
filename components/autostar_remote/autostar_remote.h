#ifndef AUTOSTAR_REMOTE_H
#define AUTOSTAR_REMOTE_H

#include "esphome.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace autostar_remote {

class AutoStarRemote : public PollingComponent, public uart::UARTDevice {
 public:
  AutoStarRemote(uart::UARTComponent *parent, text_sensor::TextSensor *lcd);

  void setup() override;
  void update() override;
  void loop() override;

  // Volite¾né z YAML lambda / services
  void send_command(const std::string &cmd);

  // Volaná z YAML uart.on_data lambda: prijaté bloky dát ako std::string (obsahuje raw bytes)
  void process_uart_data(const std::string &data);

  // nastavenie auto-stop (ms)
  void set_max_hold_ms(uint32_t ms) { this->max_hold_ms_ = ms; }

 protected:
  text_sensor::TextSensor *lcd_{nullptr};

  // pomocné metódy
  std::string decode_lcd(const std::string &raw);

  // interné buffery
  std::string uart_buf_;

  // Auto-stop pre movement
  uint32_t max_hold_ms_{10000};  // default 10s
  uint32_t move_start_ms_[4] = {0,0,0,0};
  bool move_active_[4] = {false,false,false,false};

  // pomocné mapovania
  int movement_index_from_cmd(const std::string &cmd);
  const char *stop_cmd_for_index(int idx);
};

}  // namespace autostar_remote
}  // namespace esphome

extern esphome::autostar_remote::AutoStarRemote *autostar_remote_component;

#endif  // AUTOSTAR_REMOTE_H