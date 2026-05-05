/**
 * @brief Interface of the the sensor temperature module.
 */
#ifndef SENTEM_SENSOR_TEMPERATURE_H_
#define SENTEM_SENSOR_TEMPERATURE_H_

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor temperature module.
 */
void sentem_Init(const sencty_TemperatureSensorConfiguration sensor_configuration);

/**
 * @brief Read the data from the sensor.
 */
senaty_TemperatureSensorReading sentem_ReadData(void);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENTEM_SENSOR_TEMPERATURE_H_