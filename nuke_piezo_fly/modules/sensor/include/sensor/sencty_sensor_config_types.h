/**
 * @file sencty_sensor_config_types.h
 *
 * @addtogroup sensor_config_types
 * @{
 *
 * @brief Type definitions of the sensor configuration.
 *
 * This module defines the data types and data structures used for the configuration of sensors.
 */
#ifndef SENCTY_SENSOR_CONFIG_TYPES_H_
#define SENCTY_SENSOR_CONFIG_TYPES_H_

#include <stdint.h>

/**
 * @brief Struct for the configuration of a DHT11 temperature sensor.
 */
typedef struct {
  /**
   * @brief GPIO pin of the DHT11 data line.
   */
  uint8_t data_gpio;
} sencty_TemperatureSensorConfiguration;

/**
 * @brief Struct for the configuration of an IMU sensor.
 */
typedef struct {
  /**
    * @brief I2C address of the sensor.
    */ 
   uint8_t i2c_address;
  /**
   * @brief I2C port of the sensor.
   */
  uint8_t i2c_port;
  /**
   * @brief I2C SCL GPIO pin of the sensor.
   */  
  uint8_t i2c_scl_gpio;
  /** 
   * @brief I2C SCL clock speed in Hz.
   */
  uint32_t i2c_clock_speed_hz;
  /**
   * @brief I2C SDA GPIO pin of the sensor.
   */  
  uint8_t i2c_sda_gpio;
} sencty_IMUSensorConfiguration;

/**
 * @brief Struct for the configuration of a GNSS sensor.
 */
typedef struct {
  /**
   * @brief UART port of the GNSS sensor.
   */
  uint8_t uart_port;
  /**
   * @brief UART TX GPIO pin of the GNSS sensor.
   */
  uint8_t uart_tx_gpio;
  /**
   * @brief UART RX GPIO pin of the GNSS sensor.
   */
  uint8_t uart_rx_gpio;
  /**
   * @brief UART baud rate in bit/s.
   */
  uint32_t uart_baud_rate_hz;
} sencty_GNSSSensorConfiguration;

/**
 * @brief Struct for the configuration of all sensors.
 */
typedef struct {
  /**
   * @brief Configuration of a temperature sensor.
   */
  sencty_TemperatureSensorConfiguration temperature_sensor;
  /**
   * @brief Configuration of an imu sensor.
   */
  sencty_IMUSensorConfiguration imu_sensor;
  /**
   * @brief Configuration of a GNSS sensor.
   */
  sencty_GNSSSensorConfiguration gnss_sensor;
} sencty_SensorConfiguration;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENCTY_SENSOR_CONFIG_TYPES_H_
