#include "sencfg_sensor_config.h"
#include "stocfg_storage_config.h"

#include <stdbool.h>

#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_err.h"
#include "esp_log.h"

#include "sensor/senapi_sensor_api.h"
#include "sensor/senaty_sensor_api_types.h"
#include "storage/stoapi_storage_api.h"

#define NUMBER_OF_RTC_SENSORS_READING (10U) ///< Maximum number of sensors reading saved in RTC memory.
#define MICROSECONDS_PER_SECOND (1000000U)  ///< Microseconds per second [µs].

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "MAIN";

/**
 * @brief Cycle interval of the program [µs].
 */
static const uint64_t kCycleInterval = MICROSECONDS_PER_SECOND * 1U;

/**
 * @brief Indicates that the ESP was powered on and did not wakeup from a deep sleep.
 */
RTC_DATA_ATTR static bool power_boot = true;

/**
 * @brief Number of deep sleep/ wake cycles.
 * 
 * Valid range: 0 <= value < NUMBER_OF_RTC_SENSORS_READING.
 */
RTC_DATA_ATTR static uint8_t boot_counter = 0;

/**
 * @brief Array holding sensors reading between deep sleep/ wake cycles.
 */
RTC_DATA_ATTR static senaty_SensorsReading sensors_readings[NUMBER_OF_RTC_SENSORS_READING] = {0};

void handle_bt(void);
void handle_sensors(void);

void app_main(void) {
  ESP_LOGI(kLoggerTag, "Starting %d...", boot_counter+1);

  if (power_boot) {
    ESP_LOGI(kLoggerTag, "Started from power boot");
    handle_bt();
    power_boot = false;
  } else {
    ESP_LOGI(kLoggerTag, "Started from deep sleep");
    handle_sensors();
  }

  const esp_err_t kEnableTimerWakeupResult = esp_sleep_enable_timer_wakeup(kCycleInterval);
  if (kEnableTimerWakeupResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to enable wakeup by timer: %s", esp_err_to_name(kEnableTimerWakeupResult));
  } else {
    ESP_LOGI(kLoggerTag, "Starting deep sleep...");
    esp_deep_sleep_start();
  }
  
  ESP_LOGI(kLoggerTag, "Stopping...");
}

void handle_bt(void) {
  // TODO: 3 Minuten auf BT Client warten wenn abgelaufen return
  // Wenn client connected daten aus storage lesen und übertragen dann return.
  uint8_t idx = 0;
  while (idx <= 100)
  {
    ++idx;
    ESP_LOGE(kLoggerTag, "HANDLE BT");
  }
}

void handle_sensors(void) {
  if (boot_counter < NUMBER_OF_RTC_SENSORS_READING) {
    senapi_Init(sencfg_sensor_configuration);
    const senaty_SensorsReading kSensorsReading = senapi_ReadData();

    ESP_LOGI(kLoggerTag, "Saving sensors reading in RTC memory");
    sensors_readings[boot_counter] = kSensorsReading;
    ++boot_counter;

    senapi_Deinit();
  } else {
    ESP_LOGI(kLoggerTag, "Saved %u sensors readings in RTC memory. Saving in flash memory.", NUMBER_OF_RTC_SENSORS_READING);

    stoapi_Init(stocfg_storage_configuration);
    stoapi_WriteToStorage(sensors_readings, NUMBER_OF_RTC_SENSORS_READING * sizeof(senaty_SensorsReading));

    boot_counter = 0;

    stoapi_Deinit();
  }
}

// void test_nvs_read(void) {
//   stoapi_Init(stocfg_storage_configuration);

//   size_t data_size = 0;
//   void *data = stoapi_ReadFromStorage(&data_size);
//   if (data == NULL) {
//     stoapi_ClearStorage();
//     stoapi_Deinit();
//     return;
//   } else if ((data_size == 0) || (data_size % sizeof(senaty_SensorsReading) != 0)) {
//     free(data);
//     stoapi_ClearStorage();
//     stoapi_Deinit();
//     return;
//   } else {
//     // Nothing to do here.
//   }

//   const senaty_SensorsReading *kSensorsReadings =(const senaty_SensorsReading *)data;
//   const size_t kNumberOfSensorsReadings = data_size / sizeof(senaty_SensorsReading);

//   for (size_t buffer_index = 0; buffer_index < kNumberOfSensorsReadings; ++buffer_index) {
//     const senaty_SensorsReading kSensorsReading = kSensorsReadings[buffer_index];
//     ESP_LOGI(kLoggerTag, "Read sensors data:");
//     ESP_LOGI(kLoggerTag, "\tTemperature: %i", kSensorsReading.temperature_sensor.temperature);
//     // ESP_LOGI(kLoggerTag, "\tHumidity: %u", kSensorsReading.humidity);
//     // ESP_LOGI(kLoggerTag, "\tAcceleration X: %i", kSensorsReading.acceleration_x);
//     // ESP_LOGI(kLoggerTag, "\tAcceleration Y: %i", kSensorsReading.acceleration_y);
//     // ESP_LOGI(kLoggerTag, "\tAcceleration Z: %i", kSensorsReading.acceleration_z);
//     // ESP_LOGI(kLoggerTag, "\tGyroscope X: %i", kSensorsReading.gyroscope_x);
//     // ESP_LOGI(kLoggerTag, "\tGyroscope Y: %i", kSensorsReading.gyroscope_y);
//     // ESP_LOGI(kLoggerTag, "\tGyroscope Z: %i", kSensorsReading.gyroscope_z);
//     // ESP_LOGI(kLoggerTag, "\tLongitude: %u", kSensorsReading.longitude);
//     // ESP_LOGI(kLoggerTag, "\tLatitude: %u", kSensorsReading.latitude);
//     // ESP_LOGI(kLoggerTag, "\tTimestamp: %u", kSensorsReading.timestamp);
//   }

//   free(data);
//   stoapi_ClearStorage();
//   stoapi_Deinit();
// } 