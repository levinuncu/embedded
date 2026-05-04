/**
 * @file senaty_sensor_api_types.h
 *
 * @addtogroup sensor_api_types
 * @{
 *
 * @brief Type definitions of the sensor api.
 *
 * This module defines the data types and data structures used by the sensor API interface.
 */
#ifndef SENATY_SENSOR_API_TYPES_H_
#define SENATY_SENSOR_API_TYPES_H_

#include <stdint.h>

/**
 * @brief Struct for the reading of all sensors.
 */
typedef struct {
  /**
   * @brief Rotation speed of X axis [°/s].
   * 
   * A value of INT16_MAX indicates that the position could not be read.
   */
  int16_t gyroscope_x;
  /**
   * @brief Rotation speed of Y axis [°/s].
   * 
   * A value of INT16_MAX indicates that the position could not be read.
   */
  int16_t gyroscope_y;
  /**
   * @brief Rotation speed of Z axis [°/s].
   * 
   * A value of INT16_MAX indicates that the position could not be read.
   */
  int16_t gyroscope_z;
  /**
   * @brief Acceleration of X axis [m/s²].
   * 
   * A value of INT8_MAX indicates that the position could not be read.
   */
  int8_t acceleration_x;
  /**
   * @brief Acceleration of Y axis [m/s²].
   * 
   * A value of INT8_MAX indicates that the position could not be read.
   */
  int8_t acceleration_y;
  /**
   * @brief Acceleration of Z axis [m/s²].
   * 
   * A value of INT8_MAX indicates that the position could not be read.
   */
  int8_t acceleration_z;
  /**
   * @brief Read longitude position with scaling factor 10000.
   * 
   * The MSB bit indicates if the position is west or east from the meridian.
   * A value of UINT32_MAX indicates that the position could not be read.
   */
  uint32_t longitude;
  /**
   * @brief Read latitude position with scaling factor 10000.
   * 
   * The MSB bit indicates if the position is north or south from the equator.
   * A value of UINT32_MAX indicates that the position could not be read.
   */
  uint32_t latitude;
  /**
   * @brief Read humidity of the temperature sensor [%].
   * 
   * A value of UINT8_MAX indicates that the temperature could not be read.
   */
  uint8_t humidity;
  /**
   * @brief Read temperature of the temperature sensor [°C].
   * 
   * A value of INT8_MIN indicates that the temperature could not be read.
   */
  int8_t temperature;
  /**
   * @brief Timestamp of the reading [ms].
   */
  uint32_t timestamp;
} senaty_SensorsReading;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENATY_SENSOR_API_TYPES_H_
