/**
 * @file senctr_sensor_control.c
 *
 * @brief Implementation of the sensor control.
 */
#include "senctr_sensor_control.h"

#include "sensor/senapi_sensor_api.h"
#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"
#include "common/comdef_common_definitions.h"
#include "common/comsys_common_system_adapter.h"
#include "esp_log.h"

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "SensorControl";

void senctr_Init(const sencty_SensorConfiguration *const sensor_configuration) {
  const comdef_ReturnCode kResult = senapi_Init(sensor_configuration);
  if (comdef_kNoError != kResult) {
    ESP_LOGE(kLoggerTag, "Failed to initialize sensor module");
    comsys_FatalError(kResult);
  }

  ESP_LOGI(kLoggerTag, "Sensor module initialized");
}

senaty_SensorsReading senctr_ReadSensors(void) {
  senaty_SensorsReading sensors_reading = {0};
  const comdef_ReturnCode kResult = senapi_ReadSensors(&sensors_reading);
  if (comdef_kNoError != kResult) {
    ESP_LOGE(kLoggerTag, "Failed to read sensors");
    comsys_FatalError(kResult);
  }

  ESP_LOGI(kLoggerTag, "Read sensors data:");
  ESP_LOGI(kLoggerTag, "\tTemperature: %i", sensors_reading.temperature);
  ESP_LOGI(kLoggerTag, "\tTimestamp: %u", sensors_reading.timestamp);

  return sensors_reading;
}