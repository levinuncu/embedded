#include "ext_i2c.h"
#include "mpu6050.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

static const char *TAG = "EXT_I2C";

/* Initialisiert den I2C-Bus mit den übergebenen Pins und gibt die definierten Handles für Bus und Gerät zurück. 
*   Intern wird ein neuer Master-Bus initialisiert und das MPU6050-Gerät mit der Adresse 0x68 zum Bus hinzugefügt.
*/
static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle, int sda_pin, int scl_pin)
{
    if (sda_pin < 0 || scl_pin < 0) {
        ESP_LOGE(TAG, "Invalid I2C pin numbers: SDA=%d, SCL=%d", sda_pin, scl_pin);
        return;
    }

    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,              // Dem I2C-Bus stehen 2 Ports zur Verfügung: I2C_NUM_0 und I2C_NUM_1. Hier wird I2C_NUM_0 verwendet.
        .sda_io_num = sda_pin,          
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,  // Nimmt die Standard-Clockquelle
        .glitch_ignore_cnt = 0              // Keine Glitch-Filterung, die Störungen auf der Leitungen filtern könnte
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, bus_handle));
    ESP_LOGI(TAG, "I2C bus created (SDA=%d, SCL=%d)", sda_pin, scl_pin);

    i2c_device_config_t mpu6050_dev_cfg = {
        .device_address = 0x68,            // I2C-Adresse des MPU6050, die üblicherweise 0x68 ist, wenn AD0-Pin auf GND liegt 
        .scl_speed_hz = 100000,             // I2C-Geschwindigkeit von 100 kHz
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &mpu6050_dev_cfg, dev_handle));
    ESP_LOGI(TAG, "MPU6050 device added to I2C bus at address 0x68");
}

/* Vereinfachte Schnittstelle für die Main-Funktion: Initialisiert den I2C-Bus und fügt ein MPU6050-Gerät hinzu */
void i2c_init(int sda_pin, int scl_pin, i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_init(bus_handle, dev_handle, sda_pin, scl_pin);
    ESP_LOGI(TAG, "I2C initialized successfully");
}
