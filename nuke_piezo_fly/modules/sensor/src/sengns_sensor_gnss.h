/**
 * @brief Interface of the the sensor gnss module.
 */
#ifndef SENGNS_SENSOR_GNSS_H_
#define SENGNS_SENSOR_GNSS_H_

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor gnss module.
 */
void sengns_Init(const sencty_GnssSensorConfiguration sensor_configuration);

/**
 * @brief Read the data from the sensor.
 */
senaty_GnssSensorReading sengns_ReadData(void);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENGNS_SENSOR_GNSS_H_