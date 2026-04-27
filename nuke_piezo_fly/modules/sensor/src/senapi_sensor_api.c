/**
 * @file senapi_sensor_api.c
 *
 * @brief Implementation of the sensor API.
 */

#include "sensor/senapi_sensor_api.h"

#include <stdbool.h>
#include <stddef.h>

#include "common/comass_common_assert.h"
#include "common/comdef_common_definitions.h"
#include "driver/gpio.h"
#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"
#include "senimu_sensor_imu.h"
#include "sennav_sensor_gnss.h"
#include "sentem_sensor_temperature.h"
#include "esp_timer.h"

/**
 * @brief Initialization state of the module.
 */
static bool senapi_initialized = false;

/** @addtogroup sensor_api
 * @{
 */

/**
 * @brief Checks if a sensor configuration is valid.
 *
 * This function is used to check if a sensor configuration is valid or not. Therefore the configuration is checked if all parameter are within the defined
 * ranges. All ranges for the sensor configuration are described in ::sencty_SensorConfiguration.
 *
 * @param [in] sensor_configuration Pointer to the sensor configuration. If the pointer is NULL, a ::comdef_kInvalidParameter fatal error is thrown.
 * @return true -> configuration is valid.
 * @return false -> configuration is invalid.
 */
static bool IsConfigurationValid(const sencty_SensorConfiguration *const sensor_configuration);

/** @}*/

comdef_ReturnCode senapi_Init(const sencty_SensorConfiguration *const sensor_configuration) {
  comdef_ReturnCode return_code = comdef_kNoError;

  if (sensor_configuration == NULL) {
    return_code = comdef_kInvalidParameter;
  } else if (senapi_initialized) {
    return_code = comdef_kAlreadyInitialized;
  } else if (!IsConfigurationValid(sensor_configuration)) {
    return_code = comdef_kInvalidConfiguration;
  } else {
    sentem_Init(sensor_configuration->temperature_sensor);
    senimu_Init(sensor_configuration->imu_sensor);
    sennav_Init(sensor_configuration->gnss_sensor);

    senapi_initialized = true;
  }

  return return_code;
}

comdef_ReturnCode senapi_ReadSensors(senaty_SensorsReading *const sensors_reading) {
  comdef_ReturnCode return_code = comdef_kNoError;

  if (sensors_reading == NULL) {
    return_code = comdef_kInvalidParameter;
  } else if (!senapi_initialized) {
    return_code = comdef_kNotInitialized;
  } else {
    dht11_measurement_t sentem_data = {0};
    if (sentem_ReadData(&sentem_data)) {
      sensors_reading->temperature = (int8_t)sentem_data.temperature;
      sensors_reading->humidity = sentem_data.humidity;
    } else {
      sensors_reading->temperature = INT8_MIN;
      sensors_reading->humidity = UINT8_MAX;
      return_code = comdef_kInternalError;
    }
    
    senimu_ImuData senimu_data = senimu_ReadIMUData();
    sensors_reading->acceleration_x = senimu_data.acceleration_x;
    sensors_reading->acceleration_y = senimu_data.acceleration_y;
    sensors_reading->acceleration_z = senimu_data.acceleration_z;
    sensors_reading->gyroscope_x = senimu_data.gyroscope_x;
    sensors_reading->gyroscope_y = senimu_data.gyroscope_y;
    sensors_reading->gyroscope_z = senimu_data.gyroscope_z;

        sennav_GnssData sennav_data = {0};
        if (sennav_ReadData(&sennav_data)) {
          sensors_reading->longitude = sennav_data.longitude;
          sensors_reading->latitude = sennav_data.latitude;
        } else {
          sensors_reading->longitude = UINT32_MAX;
          sensors_reading->latitude = UINT32_MAX;
          return_code = comdef_kInternalError;
        }

        sensors_reading->timestamp = (uint32_t)(esp_timer_get_time() / 1000);
  }

  return return_code;
}

static bool IsConfigurationValid(const sencty_SensorConfiguration *const sensor_configuration) {
  comass_AssertNotNull(sensor_configuration, comdef_kInvalidParameter);

  bool valid_config = true;

  if (!GPIO_IS_VALID_GPIO((gpio_num_t)sensor_configuration->temperature_sensor.data_gpio)) {
    valid_config = false;
  }

  if (!GPIO_IS_VALID_GPIO((gpio_num_t)sensor_configuration->imu_sensor.i2c_scl_gpio)) {
    valid_config = false;
  }

  if (!GPIO_IS_VALID_GPIO((gpio_num_t)sensor_configuration->imu_sensor.i2c_sda_gpio)) {
    valid_config = false;
  }

  if (sensor_configuration->imu_sensor.i2c_clock_speed_hz == 0) {
    valid_config = false;
  }

  if (!GPIO_IS_VALID_GPIO((gpio_num_t)sensor_configuration->gnss_sensor.uart_tx_gpio)) {
    valid_config = false;
  }

  if (!GPIO_IS_VALID_GPIO((gpio_num_t)sensor_configuration->gnss_sensor.uart_rx_gpio)) {
    valid_config = false;
  }

  if (sensor_configuration->gnss_sensor.uart_baud_rate_hz == 0) {
    valid_config = false;
  }

  return valid_config;
}
