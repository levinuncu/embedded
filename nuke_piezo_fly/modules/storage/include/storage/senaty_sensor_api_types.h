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
   * @brief Read temperature of the temperature sensor [°C].
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
