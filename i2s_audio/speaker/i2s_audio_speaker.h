#pragma once

#ifdef USE_ESP32

#include "../i2s_audio.h"

#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "esphome/components/speaker/speaker.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace i2s_audio {

static const size_t BUFFER_SIZE = 1024;

static const uint8_t VOLUME_TABLE[22] = {0,  1,  2,  3,  4,  6,  8,  10, 12, 14, 17,
                                         20, 23, 27, 30, 34, 38, 43, 48, 52, 58, 64}; //22 elements

enum class TaskEventType : uint8_t {
  STARTING = 0,
  STARTED,
  STOPPING,
  STOPPED,
  WARNING = 255,
};

struct TaskEvent {
  TaskEventType type;
  esp_err_t err;
};

struct DataEvent {
  bool stop;
  size_t len;
  uint8_t data[BUFFER_SIZE];
};

enum AudioFormats {
  SIGNED_INTEGER_8 = 0,
  SIGNED_INTEGER_16
};

class I2SAudioSpeaker : public I2SAudioOut, public speaker::Speaker, public Component {
 public:
  float get_setup_priority() const override { return esphome::setup_priority::LATE; }

  void setup() override;
  void loop() override;

  void set_timeout(uint32_t ms) { this->timeout_ = ms; }
  void set_dout_pin(uint8_t pin) { this->dout_pin_ = pin; }
#if SOC_I2S_SUPPORTS_DAC
  void set_internal_dac_mode(i2s_dac_mode_t mode) { this->internal_dac_mode_ = mode; }
#endif

  void start() override;
  void stop() override;
  void finish() override;

  size_t play(const uint8_t *data, size_t length) override;

  bool has_buffered_data() const override;
  
  void set_volume(float volume);
  float get_volume();
  
  void set_audio_format(AudioFormats format) { this->audio_format_ = format; }

 protected:
  void start_();
  void stop_(bool wait_on_empty);
  void watch_();

  static void player_task(void *params);

  TaskHandle_t player_task_handle_{nullptr};
  QueueHandle_t buffer_queue_;
  QueueHandle_t event_queue_;

  uint32_t timeout_{0};
  uint8_t dout_pin_{0};
  bool task_created_{false};
  
  uint8_t volume_{0};
  uint8_t m_vol_{64};
  AudioFormats audio_format_{SIGNED_INTEGER_16};

  bool buffer_wait_{false};
  const uint8_t *buffer_data_{0};
  size_t buffer_remaining_{0};
  size_t buffer_index_{0};

#if SOC_I2S_SUPPORTS_DAC
  i2s_dac_mode_t internal_dac_mode_{I2S_DAC_CHANNEL_DISABLE};
#endif
};

}  // namespace i2s_audio
}  // namespace esphome

#endif  // USE_ESP32
