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

#include <stdbool.h>
#include <stdint.h>

#include "sensor/sencty_sensor_config_types.h"

/**
 * @brief Raw DHT11 measurement data.
 */
typedef struct {
	uint8_t humidity;
	uint8_t temperature;
} dht11_measurement_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor temperature module.
 *
 * This function is used to initialize the DHT11 temperature module. A fatal error is raised, if this function is called multiple times.
 *
 * @pre The temperature module must not be initialized, otherwise a ::comdef_kAlreadyInitialized fatal error is thrown.
 * 
 * @param [in] configuration Configuration of the sensor.
 */
void sentem_Init(const sencty_TemperatureSensorConfiguration configuration);

/**
 * @brief Read the current DHT11 data.
 * 
 * This function reads the raw DHT11 measurement.
 *
 * @pre The temperature module must be initialized, otherwise a ::comdef_kNotInitialized fatal error is thrown.
 *
 * @param [out] reading Output DHT11 measurement data.
 * @return true -> data was read successfully
 * @return false -> read failed
 */
bool sentem_ReadData(dht11_measurement_t *const reading);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENTEM_SENSOR_TEMPERATURE_H_
