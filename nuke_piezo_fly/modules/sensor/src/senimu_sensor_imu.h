/**
 * @brief Interface of the the sensor imu module.
 */
#ifndef SENIMU_SENSOR_IMU_H_
#define SENIMU_SENSOR_IMU_H_

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor imu module.
 */
void senimu_Init(const sencty_ImuSensorConfiguration sensor_configuration);

/**
 * @brief Deinitialize the sensor imu module.
 */
void senimu_Deinit(void);

/**
 * @brief Read the data from the sensor.
 */
senaty_ImuSensorReading senimu_ReadData(void);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENIMU_SENSOR_IMU_H_