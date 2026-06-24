/**
 * @brief Interface of the the sensor current module.
 */
#ifndef SENTEM_SENSOR_CURRENT_H_
#define SENTEM_SENSOR_CURRENT_H_

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor current module.
 */
void sencur_Init(void);

/**
 * @brief Read the data from the sensor.
 */
senaty_CurrentSensorReading sencur_ReadData(void);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENTEM_SENSOR_CURRENT_H_