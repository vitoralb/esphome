#include "weatherservice.h"
#include "tuya.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tuya {

static const char *const TAG = "weatherservice";

WeatherService::WeatherService(Tuya *parent) : parent_(parent) {
  ESP_LOGD(TAG, "WeatherService created.");
}

void WeatherService::start(const std::vector<std::string> &requested_params) {
  this->requested_parameters_ = requested_params;
  ESP_LOGD(TAG, "WeatherService started.");
}

void WeatherService::set_weather_data_int(const std::string &key, uint32_t value) {
  ESP_LOGD(TAG, "Setting weather data (int) for key '%s': %u", key.c_str(), value);
  this->weather_data_[key] = value;
}

void WeatherService::set_weather_data_string(const std::string &key, const std::string &value) {
  ESP_LOGD(TAG, "Setting weather data (string) for key '%s': %s", key.c_str(), value.c_str());
  this->weather_data_[key] = value;
}

void WeatherService::send_weather_data() {
  if (this->requested_parameters_.empty()) {
    ESP_LOGV(TAG, "No weather parameters requested by MCU, skipping send.");
    return;
  }

  std::vector<uint8_t> payload;
  payload.push_back(0x01);  // Magic Byte

  ESP_LOGV(TAG, "Assembling weather data payload...");

  for (const auto &key : this->requested_parameters_) {
    auto it = this->weather_data_.find(key);
    if (it == this->weather_data_.end()) {
      ESP_LOGW(TAG, "Weather data for key '%s' requested by MCU but not available.", key.c_str());
      continue;
    }

    // 1. key_length
    payload.push_back(key.length());
    // 2. key_string
    payload.insert(payload.end(), key.begin(), key.end());

    const auto &value_variant = it->second;
    if (std::holds_alternative<uint32_t>(value_variant)) {
      uint32_t value = std::get<uint32_t>(value_variant);
      // 3. data_type (Integer)
      payload.push_back(0x00);
      // 4. data_length
      payload.push_back(0x04);
      // 5. data (4-byte big-endian)
      payload.push_back((value >> 24) & 0xFF);
      payload.push_back((value >> 16) & 0xFF);
      payload.push_back((value >> 8) & 0xFF);
      payload.push_back(value & 0xFF);
    } else if (std::holds_alternative<std::string>(value_variant)) {
      const std::string &value = std::get<std::string>(value_variant);
      // 3. data_type (String)
      payload.push_back(0x01);
      // 4. data_length
      payload.push_back(value.length());
      // 5. data
      payload.insert(payload.end(), value.begin(), value.end());
    }
  }

  if (payload.size() > 1) {  // Only send if we have at least one record
    ESP_LOGV(TAG, "Sending weather data payload: %s", format_hex_pretty(payload).c_str());
    this->parent_->send_command_(TuyaCommand{.cmd = TuyaCommandType::WEATHER_DATA, .payload = payload});
  } else {
    ESP_LOGV(TAG, "No available weather data for requested parameters. Not sending.");
  }
}

}  // namespace tuya
}  // namespace esphome

