/**
 * @file senimu_sensor_imu.h
 *
 * @addtogroup sensor_imu
 * @{
 *
 * @brief Interface of the the sensor imu module.
 *
 * This module provides all needed functionality for imu sensors.
 */
#ifndef SENIMU_SENSOR_IMU_H_
#define SENIMU_SENSOR_IMU_H_

#include <stdint.h>
#include <stdbool.h>

#include "sensor/sencty_sensor_config_types.h"

#define SENIMU_INVALID_ACCELERATION (INT8_MAX)
#define SENIMU_INVALID_GYROSCOPE (INT16_MAX)

/**
 * @brief Reading of the IMU sensor.
 */
typedef struct {
	/**
	 * @brief Acceleration of X axis in g.
	 */
	int8_t acceleration_x;
	/**
	 * @brief Acceleration of Y axis in g.
	 */
	int8_t acceleration_y;
	/**
	 * @brief Acceleration of Z axis in g.
	 */
	int8_t acceleration_z;
	/**
	 * @brief Gyroscope of X axis in degrees per second.
	 */
	int16_t gyroscope_x;
	/**
	 * @brief Gyroscope of Y axis in degrees per second.
	 */
	int16_t gyroscope_y;
	/**
	 * @brief Gyroscope of Z axis in degrees per second.
	 */
	int16_t gyroscope_z;
} senimu_ImuData;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the sensor imu module.
 *
 * This function is used to initialize the imu module. A fatal error is raised, if this function is called multiple times.
 *
 * @pre The imu module must not be initialized, otherwise a ::comdef_kAlreadyInitialized fatal error is thrown.
 * 
 * @param [in] configuration Configuration of the sensor.
 */
void senimu_Init(const sencty_IMUSensorConfiguration configuration);

/**
 * @brief Read the current imu data.
 * 
 * This functions reads the current imu data.
 *
 * @pre The imu module must be initialized, otherwise a ::comdef_kNotInitialized fatal error is thrown.
 *
 * @return bool Read imu data.
 */
bool senimu_ReadIMUData(senimu_ImuData *const reading);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENIMU_SENSOR_IMU_H_
