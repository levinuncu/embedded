#include "dht11.h"
#include "ble.h"
#include "ext_mpu6050.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "APP_MAIN";

/* Main-Funktion meiner Testanwendung*/
void app_main() {
    /* Initialisierung der Sensoren und BLE-Kommunikation */
    dht11_init(10);
    ble_init();
    mpu6050_init(6, 7);

    while (1) {
        /* Lesen der Sensordaten */
        ESP_LOGI(TAG, "Attempting DHT11 read...");
        dht11_measurement_t dht11_data;
        if (!dht11_read(&dht11_data)) {
            ESP_LOGW(TAG, "DHT11 read failed, retrying...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        mpu6050_measurement_t mpu6050_data;
        if (!mpu6050_read(&mpu6050_data)) {
            ESP_LOGW(TAG, "MPU6050 read failed, retrying...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        /* Erstellen eines Datenpakets mit den Sensordaten */
        struct sensor_packet_t packet = {
        .temperature = (float)dht11_data.temperature_int + ((float)dht11_data.temperature_dec / 10.0f),
        .humidity = (float)dht11_data.humidity_int + ((float)dht11_data.humidity_dec / 10.0f),
        .x_gyro = mpu6050_data.x_gyro,
        .y_gyro = mpu6050_data.y_gyro,
        .z_gyro = mpu6050_data.z_gyro,
        .x_accel = mpu6050_data.x_accel,
        .y_accel = mpu6050_data.y_accel,
        .z_accel = mpu6050_data.z_accel
        };

        /* Ausgabe der Sensordaten (zu Debugging-Zwecken)*/
        printf("Temperature: %.1f C, Humidity: %.1f %%\n", packet.temperature, packet.humidity);
        printf("Gyro: (%d, %d, %d), Accel: (%d, %d, %d)\n", packet.x_gyro, packet.y_gyro, packet.z_gyro, packet.x_accel, packet.y_accel, packet.z_accel);

        /* Senden der Messdaten über BLE */
        send_ble_data(&packet);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
