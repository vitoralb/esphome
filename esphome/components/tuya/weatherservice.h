#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>

#include "esphome/core/helpers.h"

namespace esphome {
namespace tuya {

// Forward declaration to break circular dependency
class Tuya;

class WeatherService {
 public:
  explicit WeatherService(Tuya *parent);

  /**
   * @brief Starts the weather service with a list of requested parameters.
   * @param requested_params A vector of strings representing the weather data keys the MCU wants.
   */
  void start(const std::vector<std::string> &requested_params);

  /**
   * @brief Sets or updates an integer weather data point.
   * @param key The weather data key (e.g., "w.temp").
   * @param value The integer value.
   */
  void set_weather_data_int(const std::string &key, uint32_t value);

  /**
   * @brief Sets or updates a string weather data point.
   * @param key The weather data key (e.g., "w.condition").
   * @param value The string value.
   */
  void set_weather_data_string(const std::string &key, const std::string &value);

  /**
  * @brief Assembles and sends the weather data payload to the Tuya MCU.
  */
  void send_weather_data();
  
protected:
  Tuya *parent_;
  std::vector<std::string> requested_parameters_;
  std::map<std::string, std::variant<uint32_t, std::string>> weather_data_;
};

}  // namespace tuya
}  // namespace esphome

