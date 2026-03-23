/**
 * @file senapi_sensor_api.h
 *
 * @addtogroup sensor_api
 * @{
 *
 * @brief Interface of the sensor API.
 * 
 * This module defines and implements the interface functions for the sensors.
 */
#ifndef SENAPI_SENSOR_API_H_
#define SENAPI_SENSOR_API_H_

#include "common/comdef_common_definitions.h"
#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor api module.
 *
 * This function is used to initialize the api module. If the api module is already initialized, a ::comdef_kAlreadyInitialized error is returned. After
 * checking the configuration for validity, it is then used to initialize the temperature module. If it is not valid, a ::comdef_kInvalidConfiguration is returned.
 * 
 * @param [in] sensor_configuration Pointer to the sensor configuration. If the pointer is NULL, a ::comdef_kInvalidParameter error is returned.
 * @return ::comdef_kNoError -> successful initialized
 * @return ::comdef_kAlreadyInitialized -> module already initialized
 * @return ::comdef_kInvalidConfiguration -> invalid configuration
 * @return ::comdef_kInvalidParameter -> invalid parameter
 */
comdef_ReturnCode senapi_Init(const sencty_SensorConfiguration *const sensor_configuration);

/**
 * @brief Read the values of all sensors.
 *
 * This function is used to read the values of all sensors. If the api module is not initialized, a ::comdef_kNotInitialized error is returned.
 * 
 * @param [out] sensors_reading Reading of all sensors. If the pointer is NULL, a ::comdef_kInvalidParameter error is returned.
 * @return ::radef_kNoError -> successful operation
 * @return ::radef_kNotInitialized -> module not initialized
 * @return ::radef_kInvalidParameter -> invalid parameter
 */
comdef_ReturnCode senapi_ReadSensors(senaty_SensorsReading *const sensors_reading);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENAPI_SENSOR_API_H_
