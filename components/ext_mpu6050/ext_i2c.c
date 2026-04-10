#include "mpu6050.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

static const char *TAG = "EXT_I2C";

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle, int sda_pin, int scl_pin)
{
    if (sda_pin < 0 || scl_pin < 0) {
        ESP_LOGE(TAG, "Invalid I2C pin numbers: SDA=%d, SCL=%d", sda_pin, scl_pin);
        return;
    }

    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 0
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, bus_handle));
    ESP_LOGI(TAG, "I2C bus created (SDA=%d, SCL=%d)", sda_pin, scl_pin);

    i2c_device_config_t mpu6050_dev_cfg = {
        .device_address = 0x68,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &mpu6050_dev_cfg, dev_handle));
    ESP_LOGI(TAG, "MPU6050 device added to I2C bus at address 0x68");
}

void i2c_init(int sda_pin, int scl_pin, i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_init(bus_handle, dev_handle, sda_pin, scl_pin);
    ESP_LOGI(TAG, "I2C initialized successfully");
}
