/**
 * @brief Type definitions of the sensor api.
 */
#ifndef SENATY_SENSOR_API_TYPES_H_
#define SENATY_SENSOR_API_TYPES_H_

#include <stdint.h>

/**
 * @brief Reading of a GNSS sensor.
 */
typedef struct {
  /**
   * @brief Longitude position with scaling factor 10000.
   * 
   * The MSB bit indicates if the position is west or east from the meridian.
   * A value of UINT32_MAX indicates that the longitude position could not be read.
   */
	uint32_t longitude;
  /**
   * @brief Latitude position with scaling factor 10000.
   * 
   * The MSB bit indicates if the position is north or south from the equator.
   * A value of UINT32_MAX indicates that the latitude position could not be read.
   */
	uint32_t latitude;
  /**
   * @brief Timestamp of the reading [ms].
   * 
   * A value of UINT64_MAX indicates that the timestamp could not be read.
   */
  uint64_t timestamp;
} senaty_GnssSensorReading;

/**
 * @brief Reading of an IMU sensor.
 */
typedef struct {
  /**
   * @brief Acceleration of X axis [m/s²].
   * 
   * A value of INT8_MAX indicates that the acceleration could not be read.
   */
  int8_t acceleration_x;
  /**
   * @brief Acceleration of Y axis [m/s²].
   * 
   * A value of INT8_MAX indicates that the acceleration could not be read.
   */
  int8_t acceleration_y;
  /**
   * @brief Acceleration of Z axis [m/s²].
   * 
   * A value of INT8_MAX indicates that the acceleration could not be read.
   */
  int8_t acceleration_z;
  /**
   * @brief Rotation speed of X axis [°/s].
   * 
   * A value of INT16_MAX indicates that the rotation speed could not be read.
   */
  int16_t gyroscope_x;
  /**
   * @brief Rotation speed of Y axis [°/s].
   * 
   * A value of INT16_MAX indicates that the rotation speed could not be read.
   */
  int16_t gyroscope_y;
  /**
   * @brief Rotation speed of Z axis [°/s].
   * 
   * A value of INT16_MAX indicates that the rotation speed could not be read.
   */
  int16_t gyroscope_z;
} senaty_ImuSensorReading;

/**
 * @brief Reading of a DHT11 temperature sensor.
 */
typedef struct {
  /**
   * @brief Humidity [%].
   * 
   * A value of UINT8_MAX indicates that the temperature could not be read.
   */
	uint8_t humidity;
  /**
   * @brief Temperature [°C].
   * 
   * A value of INT8_MAX indicates that the temperature could not be read.
   */
	int8_t temperature;
} senaty_TemperatureSensorReading;

/**
 * @brief Reading of all sensors.
 */
typedef struct {
  /**
   * @brief Reading of a GNSS sensor.
   */
  senaty_GnssSensorReading gnss_sensor;
  /**
   * @brief Reading of an IMU sensor.
   */
  senaty_ImuSensorReading imu_sensor;
  /**
   * @brief Reading of a temperature sensor.
   */
  senaty_TemperatureSensorReading temperature_sensor;
} senaty_SensorsReading;

#endif // SENATY_SENSOR_API_TYPES_H_
