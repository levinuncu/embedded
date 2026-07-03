/**
 * @brief Type definitions for the configuration of all sensors.
 */
#ifndef SENCTY_SENSOR_CONFIG_TYPES_H_
#define SENCTY_SENSOR_CONFIG_TYPES_H_

#include "hal/uart_types.h"
#include "soc/gpio_num.h"
#include "driver/i2c_types.h"

/**
 * @brief Configuration of a GNSS sensor.
 */
typedef struct {
  /**
   * @brief UART port of the GNSS sensor.
   */
  uart_port_t uart_port;
  /**
   * @brief UART TX GPIO pin of the GNSS sensor.
   */
  gpio_num_t uart_tx_gpio;
  /**
   * @brief UART RX GPIO pin of the GNSS sensor.
   */
  gpio_num_t uart_rx_gpio;
  /**
   * @brief UART baud rate [bit/s].
   */
  int uart_baud_rate_hz;
} sencty_GnssSensorConfiguration;

/**
 * @brief Configuration of an IMU sensor.
 */
typedef struct {
    /**
    * @brief I2C address of the sensor.
    */ 
   uint16_t i2c_address;
  /**
   * @brief I2C port of the sensor.
   */
  i2c_port_num_t i2c_port;
  /**
   * @brief I2C SCL GPIO pin of the sensor.
   */  
  gpio_num_t i2c_scl_gpio;
  /** 
   * @brief I2C SCL clock speed in Hz.
   */
  uint32_t i2c_clock_speed_hz;
  /**
   * @brief I2C SDA GPIO pin of the sensor.
   */  
  gpio_num_t i2c_sda_gpio;
} sencty_ImuSensorConfiguration;

/**
 * @brief Configuration of a DHT11 temperature sensor.
 */
typedef struct {
  /**
   * @brief GPIO pin of the DHT11 data line.
   */
  gpio_num_t data_gpio;
} sencty_TemperatureSensorConfiguration;

/**
 * @brief Configuration of all sensors.
 */
typedef struct {
  /**
   * @brief Configuration of a GNSS sensor.
   */
  sencty_GnssSensorConfiguration gnss_sensor;
  /**
   * @brief Configuration of an IMU sensor.
   */
  sencty_ImuSensorConfiguration imu_sensor;
  /**
   * @brief Configuration of a temperature sensor.
   */
  sencty_TemperatureSensorConfiguration temperature_sensor;
} sencty_SensorsConfiguration;

#endif // SENCTY_SENSOR_CONFIG_TYPES_H_
