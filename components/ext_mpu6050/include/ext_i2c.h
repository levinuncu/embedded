#pragma once

#include <stdint.h>
#include <driver/i2c_master.h>

/* Eigener I2C-Handler 
*   Sorgt für die Initialisierung des I2C-Busses und die Erstellung eines Geräte-Handles für den MPU6050-Sensor.
*/

void i2c_init(int sda_pin, int scl_pin, i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);