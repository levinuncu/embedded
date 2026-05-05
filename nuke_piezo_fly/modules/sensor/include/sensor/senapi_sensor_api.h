/**
 * @brief Interface of the the sensor api module.
 */
#ifndef SENAPI_SENSOR_API_H_
#define SENAPI_SENSOR_API_H_

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor api module.
 */
void senapi_Init(const sencty_SensorsConfiguration sensors_configuration);

/**
 * @brief Deinitialize the sensor api module.
 */
void senapi_Deinit(void);

/**
 * @brief Read the data from all sensor.
 */
senaty_SensorsReading senapi_ReadData(void);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENTEM_SENSOR_TEMPERATURE_H_