#include "bleapi_bluetooth_api.h"
#include "sencfg_sensor_config.h"
#include "stocfg_storage_config.h"

#include <stdbool.h>

#include "esp_attr.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_bt.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/projdefs.h"

#include "sensor/senapi_sensor_api.h"
#include "sensor/senaty_sensor_api_types.h"
#include "storage/stoapi_storage_api.h"

#define NUMBER_OF_RTC_SENSORS_READING (10U) ///< Maximum number of sensors reading saved in RTC memory.
#define SAMPLING_INTERVAL_MS (5000U)
#define BT_POLL_INTERVAL_MS (500U)

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "MAIN";

static SemaphoreHandle_t storage_mutex;

static uint8_t reading_counter = 0;
static senaty_SensorsReading sensors_readings[NUMBER_OF_RTC_SENSORS_READING] = {0};

static void sensor_task(void *arg);
static void bluetooth_task(void *arg);
static void save_buffer_to_storage(void);
static bool send_available_data_via_bt(void);

void app_main(void) {
  ESP_LOGI(kLoggerTag, "Starting...");

  storage_mutex = xSemaphoreCreateMutex();
  if (storage_mutex == NULL) {
    ESP_LOGE(kLoggerTag, "Failed to create mutex");
    return;
  }

  esp_err_t bt_result = bleapi_Init();
  if (bt_result != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to initialize bluetooth: %s", esp_err_to_name(bt_result));
    return;
  }

  xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
  xTaskCreate(bluetooth_task, "bluetooth_task", 4096, NULL, 4, NULL);
}

static void sensor_task(void *arg) {
  senapi_Init(sencfg_sensor_configuration);

  TickType_t last_wake_time = xTaskGetTickCount();

  while (true) {
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SAMPLING_INTERVAL_MS));

    const senaty_SensorsReading reading = senapi_ReadData();

    xSemaphoreTake(storage_mutex, portMAX_DELAY);

    sensors_readings[reading_counter] = reading;
    reading_counter++;

    if (reading_counter >= NUMBER_OF_RTC_SENSORS_READING) {
      ESP_LOGI(kLoggerTag, "Saving %u readings to flash", NUMBER_OF_RTC_SENSORS_READING);
      save_buffer_to_storage();
      reading_counter = 0;
    }

    xSemaphoreGive(storage_mutex);
  }

  senapi_Deinit();
}

static void bluetooth_task(void *arg) {
  while (true) {
    if (bleapi_IsClientConnected() && bleapi_IsNotifyEnabled()) {
      ESP_LOGI(kLoggerTag, "Bluetooth client connected. Sending data.");

      const bool data_sent = send_available_data_via_bt();

      if (data_sent) {
        ESP_LOGI(kLoggerTag, "Data sent. Disconnecting bluetooth client.");

        bleapi_DisconnectClient();

        while (bleapi_IsClientConnected()) {
          vTaskDelay(pdMS_TO_TICKS(100));
        }

        ESP_LOGI(kLoggerTag, "Bluetooth client disconnected.");
      }
    }

    vTaskDelay(pdMS_TO_TICKS(BT_POLL_INTERVAL_MS));
  }
}

static void save_buffer_to_storage(void) {
  stoapi_Init(stocfg_storage_configuration);

  stoapi_WriteToStorage(
      sensors_readings,
      reading_counter * sizeof(senaty_SensorsReading)
  );

  stoapi_Deinit();
}

static bool send_available_data_via_bt(void) {
  bool sent_anything = false;

  xSemaphoreTake(storage_mutex, portMAX_DELAY);

  stoapi_Init(stocfg_storage_configuration);

  size_t data_size = 0;
  void *data = stoapi_ReadFromStorage(&data_size);

  if (
      data != NULL &&
      data_size > 0 &&
      data_size % sizeof(senaty_SensorsReading) == 0
  ) {
    const senaty_SensorsReading *stored_readings =
        (const senaty_SensorsReading *)data;

    const size_t number_of_readings =
        data_size / sizeof(senaty_SensorsReading);

    ESP_LOGI(kLoggerTag, "Sending %zu stored readings via bluetooth", number_of_readings);

    bleapi_SendSensorsReadings(stored_readings, number_of_readings);

    sent_anything = true;

    free(data);
    stoapi_ClearStorage();
  } else {
    if (data != NULL) {
      free(data);
    }
  }

  if (reading_counter > 0) {
    ESP_LOGI(kLoggerTag, "Sending %u buffered readings via bluetooth", reading_counter);

    bleapi_SendSensorsReadings(sensors_readings, reading_counter);

    reading_counter = 0;
    sent_anything = true;
  }

  stoapi_Deinit();

  xSemaphoreGive(storage_mutex);

  return sent_anything;
}