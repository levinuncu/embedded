#pragma once

/* Eigener GATT-Service, der Sensor Daten in Characteristics bereitstellt */

#include <stdint.h>

extern uint16_t sensor_data_handle;

int gatt_svr_init(void);