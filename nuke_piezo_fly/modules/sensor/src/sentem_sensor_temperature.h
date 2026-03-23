/**
 * @file sentem_sensor_temperature.h
 *
 * @addtogroup sensor_temperature
 * @{
 *
 * @brief Interface of the the sensor temperature module.
 *
 * This module provides all needed functionality for temperature sensors.
 */
#ifndef SENTEM_SENSOR_TEMPERATURE_H_
#define SENTEM_SENSOR_TEMPERATURE_H_

#include <stdint.h>

#include "sensor/sencty_sensor_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor temperature module.
 *
 * This function is used to initialize the temperature module. A fatal error is raised, if this function is called multiple times.
 *
 * @pre The temperature module must not be initialized, otherwise a ::comdef_kAlreadyInitialized fatal error is thrown.
 * 
 * @param [in] configuration Configuration of the sensor.
 */
void sentem_Init(const sencty_TemperatureSensorConfiguration configuration);

/**
 * @brief Read the current temperature.
 * 
 * This functions reads the current temperature. INT8_MIN is returned, if reading the raw ADC voltage failed.
 *
 * @pre The temperature module must be initialized, otherwise a ::comdef_kNotInitialized fatal error is thrown.
 *
 * @return int8_t Read temperature [°C].
 */
int8_t sentem_ReadTemperature(void);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENTEM_SENSOR_TEMPERATURE_H_
