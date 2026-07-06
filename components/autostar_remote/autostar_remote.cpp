#include "autostar_remote.h"
#include <vector>
#include <cstring>

namespace esphome {
namespace autostar_remote {

AutoStarRemote *autostar_remote_component = nullptr;

AutoStarRemote::AutoStarRemote(uart::UARTComponent *parent, text_sensor::TextSensor *lcd)
  : PollingComponent(), uart::UARTDevice(parent), lcd_(lcd) {
  autostar_remote_component = this;
}

void AutoStarRemote::setup() {
  ESP_LOGD("autostar_remote", "setup()");
}

void AutoStarRemote::update() {
  // Periodicky pošleme požiadavku na LCD (PollingComponent zavolá podľa update_interval z YAML)
  this->send_command("ED");
}
/*
static std::string hexdump(const uint8_t *data, size_t len) {
  std::string out;
  char buf[8];
  for (size_t i = 0; i < len; ++i) {
    snprintf(buf, sizeof(buf), "%02X ", data[i]);
    out += buf;
  }
  return out;
}
*/
void AutoStarRemote::loop() {
  // Efektívne non-blocking čítanie: prečítame všetky dostupné bajty naraz
  const size_t avail = this->available();
  if (avail == 0) {
    // ak žiadne dáta, len skontrolujeme auto-stop časovače
    const uint32_t now = millis();
    for (int i = 0; i < 4; ++i) {
      if (move_active_[i] && (now - move_start_ms_[i]) > this->max_hold_ms_) {
        const char *stopcmd = stop_cmd_for_index(i);
        if (stopcmd != nullptr) {
          this->write_array(reinterpret_cast<const uint8_t*>(stopcmd), strlen(stopcmd));
          ESP_LOGW("autostar_remote", "Auto-stop for movement index %d sent: %s", i, stopcmd);
        }
        move_active_[i] = false;
        move_start_ms_[i] = 0;
      }
    }
    return;
  }

//  ESP_LOGD("autostar_remote", "UART available() = %u", (unsigned)avail);

  // Alokujeme buffer presne podľa počtu dostupných bajtov (bez opakovaných volaní read())
  std::vector<uint8_t> buf;
  buf.resize(avail);
  if (!this->read_array(buf.data(), avail)) {
    return;
  }

  const size_t read = buf.size();
/*
  ESP_LOGD("autostar_remote", "read_array() returned %u", (unsigned)read);
  if (read == 0) {
    return;
  }

  // zobrazíme hexdump a ASCII pre debug
  ESP_LOGD("autostar_remote", "RAW RX hex: %s", hexdump(buf.data(), read).c_str());
  {
    std::string ascii;
    for (size_t i = 0; i < read; ++i) {
      unsigned char c = buf[i];
      ascii.push_back((c >= 0x20 && c < 0x7f) ? c : '.');
    }
    ESP_LOGD("autostar_remote", "RAW RX ascii: %s", ascii.c_str());
  }
*/
  // Pridáme prijaté bajty do interného bufferu a spracujeme rámce podľa koncového znaku
  for (size_t i = 0; i < read; ++i) {
    char c = static_cast<char>(buf[i]);
    uart_buf_.push_back(c);

    // Ak AutoStar používa '#', CR ('\r') alebo LF ('\n') ako koniec rámca, považujeme to za ukončenie
    if (c == '#' || c == '\r' || c == '\n') {
      const std::string raw = uart_buf_;
      uart_buf_.clear();

      const std::string decoded = decode_lcd(raw);
      if (!decoded.empty()) {
        if (lcd_ != nullptr) {
          lcd_->publish_state(decoded.c_str());
        }
        ESP_LOGD("autostar_remote", "LCD (decoded): %s", decoded.c_str());
      } else {
        ESP_LOGD("autostar_remote", "Received (ignored) raw: %s", raw.c_str());
      }
    }
  }

  // Po spracovaní prijatého bloku skontrolujeme auto-stop (vykonáme len raz na spracovaný blok)
  const uint32_t now = millis();
  for (int i = 0; i < 4; ++i) {
    if (move_active_[i] && (now - move_start_ms_[i]) > this->max_hold_ms_) {
      const char *stopcmd = stop_cmd_for_index(i);
      if (stopcmd != nullptr) {
        this->write_array(reinterpret_cast<const uint8_t*>(stopcmd), strlen(stopcmd));
        ESP_LOGW("autostar_remote", "Auto-stop for movement index %d sent: %s", i, stopcmd);
      }
      move_active_[i] = false;
      move_start_ms_[i] = 0;
    }
  }
}

int AutoStarRemote::movement_index_from_cmd(const std::string &cmd) {
  if (cmd.find("Mn") != std::string::npos) return 0; // North
  if (cmd.find("Me") != std::string::npos) return 1; // West (me)
  if (cmd.find("Mw") != std::string::npos) return 2; // East (mw)
  if (cmd.find("Ms") != std::string::npos) return 3; // South
  // stop commands:
  if (cmd.find("Qn") != std::string::npos) return 0;
  if (cmd.find("Qw") != std::string::npos) return 1;
  if (cmd.find("Qe") != std::string::npos) return 2;
  if (cmd.find("Qs") != std::string::npos) return 3;
  return -1;
}

const char *AutoStarRemote::stop_cmd_for_index(int idx) {
  switch (idx) {
    case 0: return ":Qn#";
    case 1: return ":Qw#";
    case 2: return ":Qe#";
    case 3: return ":Qs#";
    default: return nullptr;
  }
}

void AutoStarRemote::send_command(const std::string &cmd) {
  // Vytvoríme správny formát pre AutoStar: ":CMD#"
  std::string out = cmd;
  if (!out.empty() && out.front() != ':')
    out.insert(out.begin(), ':');
  if (out.empty() || out.back() != '#')
    out.push_back('#');

  this->write_array(reinterpret_cast<const uint8_t*>(out.c_str()), out.size());
  ESP_LOGD("autostar_remote", "send_command: %s", out.c_str());

  // Auto-stop tracking: ak posielame start príkaz (M?), zapíšeme čas; ak stop (Q?), zrušíme
  int idx = movement_index_from_cmd(out);
  if (idx >= 0) {
    if (out.find("M") != std::string::npos) {
      move_active_[idx] = true;
      move_start_ms_[idx] = millis();
      ESP_LOGD("autostar_remote", "Movement %d started at %u ms", idx, move_start_ms_[idx]);
    } else if (out.find("Q") != std::string::npos) {
      move_active_[idx] = false;
      move_start_ms_[idx] = 0;
      ESP_LOGD("autostar_remote", "Movement %d stopped by command", idx);
    }
  }
}

std::string AutoStarRemote::decode_lcd(const std::string &raw) {
  // Základné čistenie: odstránime prefix :ED a sufix #
  std::string s = raw;
  while (!s.empty() && isspace((unsigned char)s.front())) s.erase(0,1);
  if (s.rfind(":ED", 0) == 0) {
    s = s.substr(3);
  }
  if (!s.empty() && s.back() == '#') s.pop_back();
  // Strip trailing CR/LF
  while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();

  std::string out;
  for (unsigned char c : s) {
    if (c == 0x0d) out.push_back('\n');
    else if (c >= 0x20 && c < 0x7F) out.push_back(c);
    else out.push_back(' ');
  }
  while (!out.empty() && (out.back() == '\n' || out.back() == ' ')) out.pop_back();
  return out;
}

}  // namespace autostar_remote
}  // namespace esphome