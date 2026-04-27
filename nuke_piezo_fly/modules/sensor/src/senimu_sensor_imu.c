/**
 * @file senimu_sensor_imu.c
 *
 * @brief Implementation of the sensor imu module.
 */

#include "senimu_sensor_imu.h"

#include <limits.h>
#include <stdbool.h>

#include "common/comass_common_assert.h"
#include "common/comdef_common_definitions.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "mpu6050.h"

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "SENIMU";

/**
 * @brief Default IMU data on read failure.
 */
static const senimu_ImuData kInvalidImuData = {
    .acceleration_x = -1.0f,
    .acceleration_y = -1.0f,
    .acceleration_z = -1.0f,
    .gyroscope_x = -1.0f,
    .gyroscope_y = -1.0f,
    .gyroscope_z = -1.0f,
};

static bool senimu_initialized = false;

static i2c_master_bus_handle_t senimu_i2c_bus_handle = NULL;
static i2c_master_dev_handle_t senimu_i2c_dev_handle = NULL;
static mpu6050_handle_t senimu_mpu6050_handle = NULL;

void senimu_Init(const sencty_IMUSensorConfiguration configuration) {
  comass_AssertTrue(!senimu_initialized, comdef_kAlreadyInitialized);
  comass_AssertTrue(GPIO_IS_VALID_GPIO((gpio_num_t)configuration.i2c_sda_gpio), comdef_kInvalidParameter);
  comass_AssertTrue(GPIO_IS_VALID_GPIO((gpio_num_t)configuration.i2c_scl_gpio), comdef_kInvalidParameter);
  comass_AssertTrue(configuration.i2c_clock_speed_hz > 0U, comdef_kInvalidParameter);
  comass_AssertTrue(configuration.i2c_address <= 0x7FU, comdef_kInvalidParameter);

  const i2c_master_bus_config_t kI2CMasterBusConfiguration = {
      .i2c_port = (i2c_port_num_t)configuration.i2c_port,
      .sda_io_num = (gpio_num_t)configuration.i2c_sda_gpio,
      .scl_io_num = (gpio_num_t)configuration.i2c_scl_gpio,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 0,
  };
  comass_AssertTrue(ESP_OK == i2c_new_master_bus(&kI2CMasterBusConfiguration, &senimu_i2c_bus_handle),
                    comdef_kInternalError);

  const i2c_device_config_t kI2CDeviceConfiguration = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = configuration.i2c_address,
      .scl_speed_hz = configuration.i2c_clock_speed_hz,
  };
  comass_AssertTrue(
      ESP_OK == i2c_master_bus_add_device(senimu_i2c_bus_handle, &kI2CDeviceConfiguration, &senimu_i2c_dev_handle),
      comdef_kInternalError);

  senimu_mpu6050_handle = mpu6050_create(senimu_i2c_dev_handle, configuration.i2c_address);
  comass_AssertNotNull(senimu_mpu6050_handle, comdef_kInternalError);

  comass_AssertTrue(ESP_OK == mpu6050_wake_up(senimu_mpu6050_handle), comdef_kInternalError);
  comass_AssertTrue(ESP_OK == mpu6050_config(senimu_mpu6050_handle, ACCE_FS_2G, GYRO_FS_250DPS),
                    comdef_kInternalError);

  ESP_LOGI(kLoggerTag, "IMU initialized on I2C port %u, address 0x%02X", configuration.i2c_port,
           configuration.i2c_address);

  senimu_initialized = true;
}

senimu_ImuData senimu_ReadIMUData(void) {
  comass_AssertTrue(senimu_initialized, comdef_kNotInitialized);
  comass_AssertNotNull(senimu_mpu6050_handle, comdef_kInternalError);

  mpu6050_acce_value_t acceleration = {0};
  mpu6050_gyro_value_t gyroscope = {0};

  if (ESP_OK != mpu6050_get_acce(senimu_mpu6050_handle, &acceleration)) {
    ESP_LOGE(kLoggerTag, "Failed to read acceleration values from MPU6050");
    return kInvalidImuData;
  }

  if (ESP_OK != mpu6050_get_gyro(senimu_mpu6050_handle, &gyroscope)) {
    ESP_LOGE(kLoggerTag, "Failed to read gyroscope values from MPU6050");
    return kInvalidImuData;
  }

  const senimu_ImuData kImuData = {
      .acceleration_x = acceleration.acce_x,
      .acceleration_y = acceleration.acce_y,
      .acceleration_z = acceleration.acce_z,
      .gyroscope_x = gyroscope.gyro_x,
      .gyroscope_y = gyroscope.gyro_y,
      .gyroscope_z = gyroscope.gyro_z,
  };

  return kImuData;
}