#pragma once

#include <stdint.h>
#include <driver/i2c_master.h>

void i2c_init(int sda_pin, int scl_pin, i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);