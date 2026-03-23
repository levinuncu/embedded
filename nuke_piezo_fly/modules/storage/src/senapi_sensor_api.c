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
#include "common/comhel_common_helper.h"
#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"
#include "sentem_sensor_temperature.h"

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
    sensors_reading->temperature = sentem_ReadTemperature();
    sensors_reading->timestamp = 123; // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }

  return return_code;
}

static bool IsConfigurationValid(const sencty_SensorConfiguration *const sensor_configuration) {
  comass_AssertNotNull(sensor_configuration, comdef_kInvalidParameter);

  bool valid_config = true;

  if (!comhel_IsU8InRange(sensor_configuration->temperature_sensor.adc_channel, sencty_kMinTemperatureADCChannel,
                          sencty_kMaxTemperatureADCChannel)) {
    valid_config = false;
  } else if (!comhel_IsU16InRange(sensor_configuration->temperature_sensor.supply_voltage,
                                  sencty_kMinTemperatureSupplyVoltage, sencty_kMaxTemperatureSupplyVoltage)) {
    valid_config = false;
  } else {
    // Nothing to do here.
  }

  return valid_config;
}
