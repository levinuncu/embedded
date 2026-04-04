#include "dht11.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

static const char *TAG = "DHT11";
static int s_dht_pin = -1;

static void dht11_start_signal(void) {
    gpio_set_direction(s_dht_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_dht_pin, 0);
    esp_rom_delay_us(18000);
    gpio_set_level(s_dht_pin, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(s_dht_pin, GPIO_MODE_INPUT);
}

static int dht11_read_bit(void) {
    while (gpio_get_level(s_dht_pin) == 0) {
    }
    int start = esp_timer_get_time();
    while (gpio_get_level(s_dht_pin) == 1) {
    }
    int duration = esp_timer_get_time() - start;
    return (duration > 40) ? 1 : 0;
}

static int dht11_read_byte(void) {
    int byte = 0;
    for (int i = 0; i < 8; i++) {
        byte <<= 1;
        byte |= dht11_read_bit();
    }
    return byte;
}

void dht11_init(int pin) {
    s_dht_pin = pin;
    gpio_set_direction(s_dht_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_dht_pin, 1);
}

bool dht11_read(dht11_reading_t *reading) {
    if (reading == NULL || s_dht_pin < 0) {
        return false;
    }

    dht11_start_signal();

    while (gpio_get_level(s_dht_pin) == 1) {
    }
    while (gpio_get_level(s_dht_pin) == 0) {
    }
    while (gpio_get_level(s_dht_pin) == 1) {
    }

    reading->humidity_int = dht11_read_byte();
    reading->humidity_dec = dht11_read_byte();
    reading->temperature_int = dht11_read_byte();
    reading->temperature_dec = dht11_read_byte();
    int checksum = dht11_read_byte();

    int sum = reading->humidity_int + reading->humidity_dec + reading->temperature_int + reading->temperature_dec;
    if (sum != checksum) {
        ESP_LOGE(TAG, "Checksum error");
        return false;
    }

    return true;
}