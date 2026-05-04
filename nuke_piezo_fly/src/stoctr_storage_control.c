/**
 * @file stoctr_storage_control.c
 *
 * @brief Implementation of the storage control.
 */
#include "stoctr_storage_control.h"

#include <stddef.h>

#include "storage/stoapi_storage_api.h"
#include "storage/stocty_storage_config_types.h"
#include "sensor/senaty_sensor_api_types.h"
#include "common/comdef_common_definitions.h"
#include "common/comsys_common_system_adapter.h"
#include "esp_log.h"

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "StorageControl";

void stoctr_Init(const stocty_StorageConfiguration *const storage_configuration) {
  const comdef_ReturnCode kResult = stoapi_Init(storage_configuration);
  if (comdef_kNoError != kResult) {
    ESP_LOGE(kLoggerTag, "Failed to initialize storage module");
    comsys_FatalError(kResult);
  }

  ESP_LOGI(kLoggerTag, "Storage module initialized");
}

void stoctr_WriteToStorage(const senaty_SensorsReading *const sensors_readings, const size_t number_of_sensors_readings) {
  const comdef_ReturnCode kResult = stoapi_WriteToStorage(sensors_readings, number_of_sensors_readings * sizeof(senaty_SensorsReading));
  if (comdef_kNoError != kResult) {
    ESP_LOGE(kLoggerTag, "Failed to write storage data");
    comsys_FatalError(kResult);
  }

  ESP_LOGI(kLoggerTag, "Wrote %u sensors reading", number_of_sensors_readings);
}

void stoctr_ReadFromStorage(const size_t sensors_readings_size, senaty_SensorsReading *const sensors_readings, size_t *const number_of_sensors_readings) {
  size_t data_size = 0;
  const comdef_ReturnCode kResult = stoapi_ReadFromStorage(sensors_readings_size * sizeof(senaty_SensorsReading), sensors_readings, &data_size);
  if (comdef_kNoError == kResult) {
    *number_of_sensors_readings = data_size / sizeof(senaty_SensorsReading);
    ESP_LOGI(kLoggerTag, "Read %u sensors reading", *number_of_sensors_readings);
  } else if (comdef_kBufferTooSmall == kResult) {
    ESP_LOGE(kLoggerTag, "Failed to read storage data. Buffer too small.");
    *number_of_sensors_readings = 0;
    stoctr_ClearStorage();
  } else {
    ESP_LOGE(kLoggerTag, "Failed to read storage data");
    comsys_FatalError(kResult);
  }
}

void stoctr_ClearStorage(void) {
  const comdef_ReturnCode kResult = stoapi_ClearStorage();
  if (comdef_kNoError != kResult) {
    ESP_LOGE(kLoggerTag, "Failed to clear storage data");
    comsys_FatalError(kResult);
  }

  ESP_LOGI(kLoggerTag, "Cleared storage data");
}