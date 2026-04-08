#include "dht11.h"
#include "ble.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "APP_MAIN";

/* Main-Funktion meiner Testanwendung*/
void app_main() {
    /* Initialisierung der Sensoren und BLE-Kommunikation */
    dht11_init(10);
    ble_init();

    while (1) {
        /* Lesen der Sensordaten */
        dht11_measurement_t dht11_data;
        if (!dht11_read(&dht11_data)) {
            ESP_LOGW(TAG, "DHT11 read failed, retrying...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        /* Erstellen eines Datenpakets mit den Sensordaten */
        struct sensor_packet_t packet = {
        .temperature = (float)dht11_data.temperature_int + ((float)dht11_data.temperature_dec / 10.0f),
        .humidity = (float)dht11_data.humidity_int + ((float)dht11_data.humidity_dec / 10.0f),
        };

        /* Ausgabe der Sensordaten (zu Debugging-Zwecken)*/
        printf("Temperature: %.1f C, Humidity: %.1f %%\n", packet.temperature, packet.humidity);

        /* Senden der Messdaten über BLE */
        send_ble_data(&packet);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
