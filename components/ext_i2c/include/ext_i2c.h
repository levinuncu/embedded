#pragma once

#include <stdint.h>
#include "esp_err.h"
#include <driver/i2c_master.h>

/* Eigener I2C-Handler 
*   Sorgt für die Initialisierung des I2C-Busses und die Erstellung eines Geräte-Handles für den MPU6050-Sensor.
*/

esp_err_t i2c_bus_init(int sda_pin, int scl_pin, i2c_master_bus_handle_t *bus_handle);
esp_err_t i2c_add_device(i2c_master_bus_handle_t bus_handle, uint16_t device_address, uint32_t scl_speed_hz, i2c_master_dev_handle_t *dev_handle);