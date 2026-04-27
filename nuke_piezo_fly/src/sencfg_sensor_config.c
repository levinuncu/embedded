/**
 * @file sencfg_sensor_config.c
 *
 * @brief Definition of the sensor configuration.
 */
#include "sencfg_sensor_config.h"

#include "driver/gpio.h"
#include "sensor/sencty_sensor_config_types.h"

const sencty_SensorConfiguration sencfg_sensor_configuration = {
  .temperature_sensor = {
    .data_gpio = GPIO_NUM_10,
  },
  .imu_sensor = {
    .i2c_address = 0x68,
    .i2c_port = 0,
    .i2c_scl_gpio = GPIO_NUM_7,
    .i2c_clock_speed_hz = 100000,
    .i2c_sda_gpio = GPIO_NUM_6,
  },
  .gnss_sensor = {
    .uart_port = 1,
    .uart_tx_gpio = GPIO_NUM_21,
    .uart_rx_gpio = GPIO_NUM_22,
    .uart_baud_rate_hz = 9600,
  },
};