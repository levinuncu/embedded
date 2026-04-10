#pragma once

#include <stdint.h>
#include "esp_err.h"

esp_err_t i2c_init(int sda_pin, int scl_pin);