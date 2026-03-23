/**
 * @file sencty_sensor_config_types.c
 *
 * @brief Definition of the sensor configuration min./max. range constants.
 */
#include "sensor/sencty_sensor_config_types.h"

#include <stdint.h>

const uint8_t sencty_kMinTemperatureADCChannel = 0U;

const uint8_t sencty_kMaxTemperatureADCChannel = 7U;

const uint16_t sencty_kMinTemperatureSupplyVoltage = 3300U;

const uint16_t sencty_kMaxTemperatureSupplyVoltage = 3300U;
