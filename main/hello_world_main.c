#include "dht11.h"
#include "ble.h"
#include "ext_i2c.h"
#include "ext_mpu6050.h"
#include "ext_gnss.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"

static const char *TAG = "APP_MAIN";

/* Main-Funktion meiner Testanwendung*/
void app_main() {
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_dev_handle_t mpu6050_dev = NULL;

    /* Initialisierung der Sensoren und BLE-Kommunikation */
    ble_init();
    dht11_init(10);

    if (i2c_bus_init(6, 7, &i2c_bus) != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed");
        return;
    }

    if (i2c_add_device(i2c_bus, 0x68, 100000, &mpu6050_dev) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add MPU6050 device");
        return;
    }

    mpu6050_init_with_device(mpu6050_dev);

    ESP_LOGI(TAG, "I2C devices initialized: MPU6050 (0x68)");

    gnss_uart_init(21, 22);

    while (1) {
        /* Lesen der Sensordaten */
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

        gnss_measurement_t gnss_data;
        if (!gnss_read_measurement(&gnss_data)) {
            ESP_LOGW(TAG, "GNSS read failed, retrying...");
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
            .z_accel = mpu6050_data.z_accel,
            .gnss = {
                .status = gnss_data.status,
                .datetime = {
                    .day = gnss_data.datetime.day,
                    .month = gnss_data.datetime.month,
                    .year = gnss_data.datetime.year,
                    .hours = gnss_data.datetime.hours,
                    .minutes = gnss_data.datetime.minutes,
                    .seconds = gnss_data.datetime.seconds,
                },
                .position = {
                    .lat = gnss_data.position.lat,
                    .lat_dir = gnss_data.position.lat_dir,
                    .lon = gnss_data.position.lon,
                    .lon_dir = gnss_data.position.lon_dir,
                },
                .speed = gnss_data.speed,
                .course = gnss_data.course,
            },
        };

        /* Ausgabe der Sensordaten (zu Debugging-Zwecken)*/
        printf("Temperature: %.1f C, Humidity: %.1f %%\n", packet.temperature, packet.humidity);
        printf("Gyro: (%d, %d, %d), Accel: (%d, %d, %d)\n", packet.x_gyro, packet.y_gyro, packet.z_gyro, packet.x_accel, packet.y_accel, packet.z_accel);
        printf("GNSS: Lon: %ld%c, Lat: %ld%c, Status: %c\n",
            packet.gnss.position.lon,
            packet.gnss.position.lon_dir,
            packet.gnss.position.lat,
            packet.gnss.position.lat_dir,
            packet.gnss.status);
        printf("GNSS UTC: %02u.%02u.%04u %02u:%02u:%02u\n",
            packet.gnss.datetime.day,
            packet.gnss.datetime.month,
            packet.gnss.datetime.year,
            packet.gnss.datetime.hours,
            packet.gnss.datetime.minutes,
            packet.gnss.datetime.seconds);

        /* Senden der Messdaten über BLE */
        send_ble_data(&packet);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
