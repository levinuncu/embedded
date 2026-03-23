/**
 * @file sencfg_sensor_config.c
 *
 * @brief Definition of the sensor configuration.
 */
#include "sencfg_sensor_config.h"

#include "sensor/sencty_sensor_config_types.h"
#include "hal/adc_types.h"

const sencty_SensorConfiguration sencfg_sensor_configuration = {
  .temperature_sensor = {
    .adc_channel = ADC_CHANNEL_0,
    .reference_resistance = 1000U,
    .resistance = 100U,
    .supply_voltage = 3300U,
    .temperature_coefficient = 385U,
  }
};