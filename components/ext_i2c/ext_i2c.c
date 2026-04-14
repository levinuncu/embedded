#include "ext_i2c.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

static const char *TAG = "EXT_I2C";

/* Initialisiert den I2C-Bus mit den angegebenen Pins */
esp_err_t i2c_bus_init(int sda_pin, int scl_pin, i2c_master_bus_handle_t *bus_handle)
{
    if (bus_handle == NULL) {
        ESP_LOGE(TAG, "Invalid bus handle pointer");
        return ESP_ERR_INVALID_ARG;
    }

    if (sda_pin < 0 || scl_pin < 0) {
        ESP_LOGE(TAG, "Invalid I2C pin numbers: SDA=%d, SCL=%d", sda_pin, scl_pin);
        return ESP_ERR_INVALID_ARG;
    }

    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,              // Dem I2C-Bus stehen 2 Ports zur Verfügung: I2C_NUM_0 und I2C_NUM_1. Hier wird I2C_NUM_0 verwendet.
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,  // Nimmt die Standard-Clockquelle
        .glitch_ignore_cnt = 0              // Keine Glitch-Filterung, die Störungen auf der Leitungen filtern könnte
    };

    esp_err_t err = i2c_new_master_bus(&i2c_bus_config, bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C bus: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C bus created (SDA=%d, SCL=%d)", sda_pin, scl_pin);
    return ESP_OK;
}

/* Hilfsmethode, die ein I2C-Gerät zum Bus hinzufügt */
esp_err_t i2c_add_device(i2c_master_bus_handle_t bus_handle, uint16_t device_address, uint32_t scl_speed_hz, i2c_master_dev_handle_t *dev_handle)
{
    if (bus_handle == NULL || dev_handle == NULL || scl_speed_hz == 0) {
        ESP_LOGE(TAG, "Invalid args for i2c_add_device");
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_cfg = {
        .device_address = device_address,
        .scl_speed_hz = scl_speed_hz,
    };

    esp_err_t err = i2c_master_bus_add_device(bus_handle, &dev_cfg, dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add I2C device 0x%02X: %s", device_address, esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C device added at address 0x%02X", device_address);
    return ESP_OK;
}
