/**
 * @brief Implementation of the sensor configuration.
 */
#include "sencfg_sensor_config.h"

#include "hal/uart_types.h"
#include "soc/gpio_num.h"
#include "hal/i2c_types.h"

#include "sensor/sencty_sensor_config_types.h"

const sencty_SensorsConfiguration sencfg_sensor_configuration = {
  .gnss_sensor = {
    .uart_port = UART_NUM_1,
    .uart_tx_gpio = GPIO_NUM_32,
    .uart_rx_gpio = GPIO_NUM_33,
    .uart_baud_rate_hz = 9600,
  },
  .imu_sensor = {
    .i2c_address = 0x68,
    .i2c_port = I2C_NUM_0,
    .i2c_scl_gpio = GPIO_NUM_22,
    .i2c_clock_speed_hz = 100000,
    .i2c_sda_gpio = GPIO_NUM_21,
  },
  .temperature_sensor = {
    .data_gpio = GPIO_NUM_12,
  },
};