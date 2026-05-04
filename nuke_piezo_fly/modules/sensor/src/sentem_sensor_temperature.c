/**
 * @file sentem_sensor_temperature.c
 *
 * @brief Implementation of the the sensor temperature module.
 */
#include "sentem_sensor_temperature.h"

#include <stdbool.h>

#include "common/comass_common_assert.h"
#include "common/comdef_common_definitions.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "sensor/sencty_sensor_config_types.h"

#define DHT11_RESPONSE_TIMEOUT_US (200)
#define DHT11_BIT_START_TIMEOUT_US (100)
#define DHT11_BIT_HIGH_TIMEOUT_US (100)

/**
 * @brief Initialization state of the module.
 */
static bool sentem_initialized = false;

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "DHT11";

/**
 * @brief GPIO pin used for the DHT11 data line.
 */
static int sentem_data_gpio = -1;

static bool dht11_wait_for_level(int level, int timeout_us);
static void dht11_start_signal(void);
static int dht11_read_bit(void);
static bool dht11_read_byte(uint8_t *const byte_out);

void sentem_Init(const sencty_TemperatureSensorConfiguration configuration) {
  comass_AssertTrue(!sentem_initialized, comdef_kAlreadyInitialized);
  comass_AssertTrue(GPIO_IS_VALID_GPIO(configuration.data_gpio), comdef_kInvalidParameter);

  sentem_data_gpio = (int)configuration.data_gpio;

  const gpio_config_t kGpioConfig = {
    pin_bit_mask: (1ULL << sentem_data_gpio),
    mode: GPIO_MODE_OUTPUT,
    pull_up_en: GPIO_PULLUP_ENABLE,
    pull_down_en: GPIO_PULLDOWN_DISABLE,
    intr_type: GPIO_INTR_DISABLE,
  };
	comass_AssertTrue(ESP_OK == gpio_config(&kGpioConfig), comdef_kInternalError);
	comass_AssertTrue(ESP_OK == gpio_set_level((gpio_num_t)sentem_data_gpio, 1), comdef_kInternalError);

  ESP_LOGI(kLoggerTag, "DHT11 initialized on GPIO port %u", configuration.data_gpio);

  sentem_initialized = true;
}

bool sentem_ReadData(dht11_measurement_t *const reading) {
  comass_AssertTrue(sentem_initialized, comdef_kNotInitialized);
  comass_AssertNotNull(reading, comdef_kInvalidParameter);

  reading->humidity = SENTEM_INVALID_HUMIDITY;
  reading->temperature = SENTEM_INVALID_TEMPERATURE;

  dht11_start_signal();

  if (!dht11_wait_for_level(0, DHT11_RESPONSE_TIMEOUT_US) || !dht11_wait_for_level(1, DHT11_RESPONSE_TIMEOUT_US) ||
      !dht11_wait_for_level(0, DHT11_RESPONSE_TIMEOUT_US)) {
    ESP_LOGW(kLoggerTag, "Sensor response timeout");
    return false;
  }

  uint8_t humidity_dec = 0U;
  uint8_t temperature_dec = 0U;
  uint8_t checksum = 0U;
  if (!dht11_read_byte(&reading->humidity) || !dht11_read_byte(&humidity_dec) ||
      !dht11_read_byte(&reading->temperature) || !dht11_read_byte(&temperature_dec) ||
      !dht11_read_byte(&checksum)) {
    ESP_LOGW(kLoggerTag, "Timed out while reading sensor data");
    return false;
  }

  const uint8_t kSum = (uint8_t)(reading->humidity + humidity_dec + reading->temperature + temperature_dec);
  if (kSum != checksum) {
    ESP_LOGE(kLoggerTag, "Checksum error");
    return false;
  }

  return true;
}

static bool dht11_wait_for_level(int level, int timeout_us) {
  const int64_t kStart = esp_timer_get_time();
  while (gpio_get_level((gpio_num_t)sentem_data_gpio) != level) {
    if ((esp_timer_get_time() - kStart) > timeout_us) {
      return false;
    }
  }

  return true;
}

static void dht11_start_signal(void) {
  gpio_set_direction((gpio_num_t)sentem_data_gpio, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)sentem_data_gpio, 0);
  esp_rom_delay_us(18000);
  gpio_set_level((gpio_num_t)sentem_data_gpio, 1);
  esp_rom_delay_us(30);
  gpio_set_direction((gpio_num_t)sentem_data_gpio, GPIO_MODE_INPUT);
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
