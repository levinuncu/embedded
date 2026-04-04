#include "dht11.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main() {
    dht11_init(10);

    while (1) {
        dht11_reading_t reading;

        if (dht11_read(&reading)) {
            printf("Temp: %d.%d C, Hum: %d.%d %%\n",
                   reading.temperature_int,
                   reading.temperature_dec,
                   reading.humidity_int,
                   reading.humidity_dec);
        } else {
            printf("Checksum error\n");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
