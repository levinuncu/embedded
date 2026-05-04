#include <stdint.h>
#include <stddef.h>

#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "sencfg_sensor_config.h"
#include "senctr_sensor_control.h"
#include "stocfg_storage_config.h"
#include "stoctr_storage_control.h"
#include "common/comdef_common_definitions.h"
#include "common/comsys_common_system_adapter.h"
#include "sensor/senaty_sensor_api_types.h"
/* #include "ble.h" */

#define NUMBER_OF_RTC_SENSORS_READING (10U) ///< Maximum number of sensors reading saved in RTC memory.
#define MICROSECONDS_PER_SECOND (1000000U)  ///< Microseconds per second [µs].

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "Main";

/**
 * @brief Cycle interval of the program [µs].
 */
static const uint64_t kCycleInterval = MICROSECONDS_PER_SECOND * 1U;

/**
 * @brief Number of deep sleep/ wake cycles.
 * 
 * Valid range: 0 <= value < NUMBER_OF_RTC_SENSORS_READING.
 */
RTC_DATA_ATTR static uint8_t boot_counter = 0;

/**
 * @brief Array holding sensors reading between deep sleeps.
 */
RTC_DATA_ATTR static senaty_SensorsReading sensors_readings[NUMBER_OF_RTC_SENSORS_READING] = {0};

/*
  Remove only for testing
*/
RTC_DATA_ATTR static uint8_t write_nvs_counter = 0;
void test_nvs_read(void);
/*
  Remove only for testing
*/

void app_main(void) {
  ESP_LOGI(kLoggerTag, "Starting %d...", boot_counter+1);

  /*
   * BLE integration point:
   * Initialize BLE once during startup before sensor data is sent.
   */
  // bleapi_Init();

  if (boot_counter == NUMBER_OF_RTC_SENSORS_READING) {
    ESP_LOGI(kLoggerTag, "Saved %u sensors readings. Saving in flash memory.", NUMBER_OF_RTC_SENSORS_READING);

    stoctr_Init(&stocfg_storage_configuration);
    stoctr_WriteToStorage(sensors_readings, NUMBER_OF_RTC_SENSORS_READING);

    ++write_nvs_counter;
    boot_counter = 0;
  } else {
    senctr_Init(&sencfg_sensor_configuration);
    const senaty_SensorsReading kSensorsReading = senctr_ReadSensors();
    /*
     * BLE integration point:
     * Send the current reading to a connected BLE client.
     */
    // bleapi_SendSensorsReading(&kSensorsReading);

    ESP_LOGI(kLoggerTag, "Saving sensors reading in RTC memory.");
    sensors_readings[boot_counter] = kSensorsReading;
    ++boot_counter;
  }

  if (write_nvs_counter == 2U) {
    test_nvs_read();

    write_nvs_counter = 0;
  }
  
  if (ESP_OK != esp_sleep_enable_timer_wakeup(kCycleInterval)) {
    ESP_LOGE(kLoggerTag, "Failed to enable timer wakeup");
    comsys_FatalError(comdef_kInternalError);
  }

  ESP_LOGI(kLoggerTag, "Stopping...");

  esp_deep_sleep_start();
}


void test_nvs_read(void) {
  const size_t kBufferSize = 20;
  senaty_SensorsReading buffer[kBufferSize];

  size_t number_of_sensors_readings = 0;
  stoctr_ReadFromStorage(kBufferSize, buffer, &number_of_sensors_readings); // TODO: Check if this code path for reading has any UB.
  for (size_t buffer_index = 0; buffer_index < number_of_sensors_readings; ++buffer_index) {
    const senaty_SensorsReading kSensorsReading = buffer[buffer_index];
    ESP_LOGI(kLoggerTag, "Read sensors data:");
    ESP_LOGI(kLoggerTag, "\tTemperature: %i", kSensorsReading.temperature);
    ESP_LOGI(kLoggerTag, "\tHumidity: %u", kSensorsReading.humidity);
    ESP_LOGI(kLoggerTag, "\tAcceleration X: %i", kSensorsReading.acceleration_x);
    ESP_LOGI(kLoggerTag, "\tAcceleration Y: %i", kSensorsReading.acceleration_y);
    ESP_LOGI(kLoggerTag, "\tAcceleration Z: %i", kSensorsReading.acceleration_z);
    ESP_LOGI(kLoggerTag, "\tGyroscope X: %i", kSensorsReading.gyroscope_x);
    ESP_LOGI(kLoggerTag, "\tGyroscope Y: %i", kSensorsReading.gyroscope_y);
    ESP_LOGI(kLoggerTag, "\tGyroscope Z: %i", kSensorsReading.gyroscope_z);
    ESP_LOGI(kLoggerTag, "\tLongitude: %u", kSensorsReading.longitude);
    ESP_LOGI(kLoggerTag, "\tLatitude: %u", kSensorsReading.latitude);
    ESP_LOGI(kLoggerTag, "\tTimestamp: %u", kSensorsReading.timestamp);
  }

  stoctr_ClearStorage();
} 