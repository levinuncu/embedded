#include "ext_i2c.h"
#include "esp_log.h"
#include "driver/i2c.h"

static const char *TAG = "EXT_I2C";

esp_err_t i2c_init(int sda_pin, int scl_pin)
{
    if (sda_pin < 0 || scl_pin < 0) {
        ESP_LOGE(TAG, "Invalid I2C pin numbers: SDA=%d, SCL=%d", sda_pin, scl_pin);
        return ESP_ERR_INVALID_ARG;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = scl_pin,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = 100000,
        .clk_flags = 0,
    };

    esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (err == ESP_ERR_INVALID_STATE) {
        // Driver already installed, this is fine for repeated init calls.
        ESP_LOGW(TAG, "I2C driver already installed on port %d", I2C_NUM_0);
        return ESP_OK;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C initialized successfully");
    return ESP_OK;
}
