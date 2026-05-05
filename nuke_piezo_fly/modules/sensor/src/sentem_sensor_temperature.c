/**
 * @brief Implementation of the the sensor temperature module.
 */
#include "sentem_sensor_temperature.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#define DHT11_RESPONSE_TIMEOUT_US (200)
#define DHT11_BIT_START_TIMEOUT_US (100)
#define DHT11_BIT_HIGH_TIMEOUT_US (100)

#define INVALID_HUMIDITY (UINT8_MAX) //< Value for an invalid humidity.
#define INVALID_TEMPERATURE (INT8_MAX) //< Value for an invalid temperature.

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "SENTEM";

/**
 * @brief Initialization state of the module.
 */
static bool initialized = false;

/**
 * @brief Configuration of the sensor.
 */
static sencty_TemperatureSensorConfiguration configuration;

static bool dht11_wait_for_level(int level, int timeout_us);
static void dht11_start_signal(void);
static int dht11_read_bit(void);
static bool dht11_read_byte(uint8_t *const byte_out);

void sentem_Init(const sencty_TemperatureSensorConfiguration sensor_configuration) {
  configuration = sensor_configuration;

  const gpio_config_t kGpioConfig = {
    pin_bit_mask: (1ULL << configuration.data_gpio),
    mode: GPIO_MODE_OUTPUT,
    pull_up_en: GPIO_PULLUP_ENABLE,
    pull_down_en: GPIO_PULLDOWN_DISABLE,
    intr_type: GPIO_INTR_DISABLE,
  };

  const esp_err_t kConfigResult = gpio_config(&kGpioConfig);
  if (kConfigResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to configure the GPIO: %s", esp_err_to_name(kConfigResult));
    return;
  }

  const esp_err_t kSetLevelResult = gpio_set_level(configuration.data_gpio, 1);
  if (kSetLevelResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to set the output level: %s", esp_err_to_name(kSetLevelResult));
    return;
  }

  ESP_LOGI(kLoggerTag, "Initialized");
  initialized = true;
}

senaty_TemperatureSensorReading sentem_ReadData(void) {
  const senaty_TemperatureSensorReading kFailedReading = {
    .humidity = INVALID_HUMIDITY,
    .temperature = INVALID_TEMPERATURE,
  };

  if (!initialized) {
    return kFailedReading;
  }

  dht11_start_signal();

  if (!dht11_wait_for_level(0, DHT11_RESPONSE_TIMEOUT_US) || !dht11_wait_for_level(1, DHT11_RESPONSE_TIMEOUT_US) || !dht11_wait_for_level(0, DHT11_RESPONSE_TIMEOUT_US)) {
    ESP_LOGE(kLoggerTag, "Sensor signal response timed out");
    return kFailedReading;
  }

  senaty_TemperatureSensorReading reading = {0};
  uint8_t humidity_dec, temperature_dec, checksum = 0;
  if (!dht11_read_byte(&reading.humidity) || !dht11_read_byte(&humidity_dec) || !dht11_read_byte((uint8_t*)&reading.temperature) || !dht11_read_byte(&temperature_dec) || !dht11_read_byte(&checksum)) {
    ESP_LOGE(kLoggerTag, "Sensor reading response timed out");
    return kFailedReading;
  }

  const uint8_t kCalculatedChecksum = (uint8_t)(reading.humidity + humidity_dec + reading.temperature + temperature_dec);
  if (kCalculatedChecksum != checksum) {
    ESP_LOGE(kLoggerTag, "Sensor reading has invalid checksum");
    return kFailedReading;
  }

  ESP_LOGI(kLoggerTag, "Read sensor data");
  return reading;
}

static bool dht11_wait_for_level(int level, int timeout_us) {
  const int64_t kStart = esp_timer_get_time();
  while (gpio_get_level(configuration.data_gpio) != level) {
    if ((esp_timer_get_time() - kStart) > timeout_us) {
      return false;
    }
  }

  return true;
}

static void dht11_start_signal(void) {
  gpio_set_direction(configuration.data_gpio, GPIO_MODE_OUTPUT);
  gpio_set_level(configuration.data_gpio, 0);
  esp_rom_delay_us(18000);
  gpio_set_level(configuration.data_gpio, 1);
  esp_rom_delay_us(30);
  gpio_set_direction(configuration.data_gpio, GPIO_MODE_INPUT);
}

static int dht11_read_bit(void) {
  if (!dht11_wait_for_level(1, DHT11_BIT_START_TIMEOUT_US)) {
    return -1;
  }

  const int64_t kStart = esp_timer_get_time();
  if (!dht11_wait_for_level(0, DHT11_BIT_HIGH_TIMEOUT_US)) {
    return -1;
  }

  const int64_t kDuration = esp_timer_get_time() - kStart;
  return (kDuration > 40) ? 1 : 0;
}

static bool dht11_read_byte(uint8_t *const byte_out) {
  uint8_t byte = 0U;
  for (int bit_index = 0; bit_index < 8; ++bit_index) {
    const int bit = dht11_read_bit();
    if (bit < 0) {
      return false;
    }

    byte = (uint8_t)(byte << 1);
    byte = (uint8_t)(byte | (uint8_t)bit);
  }

  *byte_out = byte;
  return true;
}
