/**
 * @file senctr_sensor_control.h
 *
 * @addtogroup sensor_control
 * @{
 *
 * @brief Interface of the sensor control.
 * 
 * This module provides functions used to control all sensors.
 */
#ifndef SENCTR_SENSOR_CONTROL_H_
#define SENCTR_SENSOR_CONTROL_H_

#include "sensor/sencty_sensor_config_types.h"
#include "sensor/senaty_sensor_api_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor control module.
 *
 * This function is used to initialize the control module. If the initialization fails, a fatal error is thrown.
 * This function is only a wrapper function, which calls senapi_Init().
 * 
 * @param [in] sensor_configuration Pointer to the sensor configuration.
 */
void senctr_Init(const sencty_SensorConfiguration *const sensor_configuration);

/**
 * @brief Read the values of all sensors.
 *
 * This function is used to read the values of all sensors. If the reading fails, a fatal error is thrown.
 * This function is only a wrapper function, which calls senapi_ReadSensors().
 *
 * @return ::senaty_SensorsReading -> Struct with the read sensor data
 */
senaty_SensorsReading senctr_ReadSensors(void);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENCTR_SENSOR_CONTROL_H_
