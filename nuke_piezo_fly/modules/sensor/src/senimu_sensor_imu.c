/**
 * @brief Implementation of the the sensor imu module.
 */
#include "senimu_sensor_imu.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "soc/clk_tree_defs.h"
#include "mpu6050.h"
#include "esp_err.h"
#include "esp_log.h"

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#define INVALID_ACCELERATION (INT8_MAX) //< Value for an invalid acceleration.
#define INVALID_GYROSCOPE (INT16_MAX) //< Value for an invalid gyroscope.

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "SENIMU";

/**
 * @brief Initialization state of the module.
 */
static bool initialized = false;

/**
 * @brief Configuration of the sensor.
 */
static sencty_ImuSensorConfiguration configuration;

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t i2c_dev_handle = NULL;
static mpu6050_handle_t mpu6050_handle = NULL;

void senimu_Init(const sencty_ImuSensorConfiguration sensor_configuration) {
  configuration = sensor_configuration;

  const i2c_master_bus_config_t kI2cMasterConfig = {
      .i2c_port = configuration.i2c_port,
      .sda_io_num = configuration.i2c_sda_gpio,
      .scl_io_num = configuration.i2c_scl_gpio,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 0,
      .flags = {
        .enable_internal_pullup = 1,
      },
  };

  const esp_err_t kI2cMasterResult = i2c_new_master_bus(&kI2cMasterConfig, &i2c_bus_handle);
  if (kI2cMasterResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to create the I2C master: %s", esp_err_to_name(kI2cMasterResult));
    return;
  }

	const i2c_device_config_t kI2cDeviceConfig = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = configuration.i2c_address,
      .scl_speed_hz = configuration.i2c_clock_speed_hz,
	};

	const esp_err_t kI2cDeviceResult = i2c_master_bus_add_device(i2c_bus_handle, &kI2cDeviceConfig, &i2c_dev_handle);
  if (kI2cDeviceResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to add the I2C device: %s", esp_err_to_name(kI2cDeviceResult));
    senimu_Deinit();
    return;
  }

	mpu6050_handle = mpu6050_create(i2c_dev_handle, configuration.i2c_address);
	if (mpu6050_handle == NULL) {
    ESP_LOGE(kLoggerTag, "Failed to create the MPU6050");
    senimu_Deinit();
    return;
	}

	const esp_err_t kWakeUpResult = mpu6050_wake_up(mpu6050_handle);
	if (kWakeUpResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to wake up the MPU6050: %s", esp_err_to_name(kWakeUpResult));
    senimu_Deinit();
    return;
	}

	const esp_err_t kConfigResult = mpu6050_config(mpu6050_handle, ACCE_FS_2G, GYRO_FS_250DPS);
	if (kConfigResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to configure the MPU6050: %s", esp_err_to_name(kConfigResult));
    senimu_Deinit();
    return;
	}

  ESP_LOGI(kLoggerTag, "Initialized");
  initialized = true;
}

void senimu_Deinit(void) {
  if (mpu6050_handle != NULL) {
    mpu6050_delete(mpu6050_handle);
    mpu6050_handle = NULL;
  }

  if (i2c_dev_handle != NULL) {
    (void)i2c_master_bus_rm_device(i2c_dev_handle);
    i2c_dev_handle = NULL;
  }

  if (i2c_bus_handle != NULL) {
    (void)i2c_del_master_bus(i2c_bus_handle);
    i2c_bus_handle = NULL;
  }

  ESP_LOGI(kLoggerTag, "Deinitialized");
  initialized = false;
}

senaty_ImuSensorReading senimu_ReadData(void) {
  const senaty_ImuSensorReading kFailedReading = {
    .acceleration_x = INVALID_ACCELERATION,
		.acceleration_y = INVALID_ACCELERATION,
		.acceleration_z = INVALID_ACCELERATION,
		.gyroscope_x = INVALID_GYROSCOPE,
		.gyroscope_y = INVALID_GYROSCOPE,
		.gyroscope_z = INVALID_GYROSCOPE,
  };

  if (!initialized) {
    return kFailedReading;
  }

	mpu6050_acce_value_t acceleration = {0};
	const esp_err_t kAccelerationResult = mpu6050_get_acce(mpu6050_handle, &acceleration);
	if (kAccelerationResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to read the acceleration: %s", esp_err_to_name(kAccelerationResult));
		return kFailedReading;
	}

  mpu6050_gyro_value_t gyroscope = {0};
	const esp_err_t kGyroscopeResult = mpu6050_get_gyro(mpu6050_handle, &gyroscope);
	if (kGyroscopeResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to read the gyroscope: %s", esp_err_to_name(kGyroscopeResult));
		return kFailedReading;
	}

  senaty_ImuSensorReading reading = {
    .acceleration_x = (int8_t)acceleration.acce_x,
		.acceleration_y = (int8_t)acceleration.acce_y,
		.acceleration_z = (int8_t)acceleration.acce_z,
		.gyroscope_x = (int16_t)gyroscope.gyro_x,
		.gyroscope_y = (int16_t)gyroscope.gyro_y,
		.gyroscope_z = (int16_t)gyroscope.gyro_z,
  };
	
  ESP_LOGI(kLoggerTag, "Read sensor data");
  return reading;
}
