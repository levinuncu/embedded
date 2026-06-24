/**
 * @brief Implementation of the the sensor temperature module.
 */
#include "sencur_sensor_current.h"

#include <stdint.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_err.h"
#include "esp_log.h"

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#define INVALID_CURRENT (INT16_MAX) //< Value for an invalid current.
#define SENSOR_MIN_VOLTAGE (0.0f)
#define SENSOR_MAX_VOLTAGE (5.0f)
#define MILLI_VOLT_PER_AMPERE (100U)
#define MILLI_VOLT_AT_ZERO_AMPERE (2500U)

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "SENCUR";

/**
 * @brief Initialization state of the module.
 */
static bool initialized = false;

static adc_oneshot_unit_handle_t adc_handle;
static adc_channel_t adc_channel = ADC_CHANNEL_2;

void sencur_Init(void) {
  adc_oneshot_unit_init_cfg_t init_cfg = {
      .unit_id = ADC_UNIT_2,
      .ulp_mode = ADC_ULP_MODE_DISABLE
  };
  const esp_err_t kCreateResult = adc_oneshot_new_unit(&init_cfg, &adc_handle);
  if (kCreateResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to create the ADC: %s", esp_err_to_name(kCreateResult));
    return;
  }

  adc_oneshot_chan_cfg_t chan_cfg = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT
  };
  const esp_err_t kConfigResult = adc_oneshot_config_channel(adc_handle, adc_channel, &chan_cfg);
  if (kConfigResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to configure the ADC: %s", esp_err_to_name(kConfigResult));
    return;
  }

  ESP_LOGI(kLoggerTag, "Initialized");
  initialized = true;
}

senaty_CurrentSensorReading sencur_ReadData(void) {
  const senaty_CurrentSensorReading kFailedReading = {
    .current = INVALID_CURRENT,
  };

  if (!initialized) {
    return kFailedReading;
  }

  int raw = 0; 
  const esp_err_t kReadResult = adc_oneshot_read(adc_handle, adc_channel, &raw);
  if (kReadResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to read the raw value: %s", esp_err_to_name(kReadResult));
    return kFailedReading;
  }

  float adc_voltage = (raw / 4095.0f) * 3.3f; // Convert read value to voltage
  float sensor_voltage = adc_voltage * ((12.0f + 22.0f) / 22.0f); // Voltage divider: U_ges = U_adc * ((R1 + R2) / R2)

  if ((sensor_voltage < SENSOR_MIN_VOLTAGE) || (sensor_voltage > SENSOR_MAX_VOLTAGE)) {
    ESP_LOGE(kLoggerTag, "Read voltage is out of range: %f", sensor_voltage);
    return kFailedReading;
  }

  uint16_t milli_volt = (uint16_t)(sensor_voltage * 1000.0f);
  int16_t milli_ampere =
      (int16_t)((int32_t)milli_volt - MILLI_VOLT_AT_ZERO_AMPERE) * 1000
      / MILLI_VOLT_PER_AMPERE;

  senaty_CurrentSensorReading reading = {
    .current = milli_ampere,
  };

  ESP_LOGI(kLoggerTag, "Read current data. Current %i", reading.current);
  return reading;
}