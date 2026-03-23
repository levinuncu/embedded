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

#include "hal/adc_types.h"

/**
 * @brief Struct for the configuration of a temperature sensor.
 */
typedef struct {
  /**
   * @brief ADC channel of the sensor.
   * 
   * Valid range: ::sencty_kMinTemperatureADCChannel <= value <= ::sencty_kMaxTemperatureADCChannel.
   */
  adc_channel_t adc_channel;
  /**
  * @brief Reference resistance of the voltage divider [ohm].
   */
  uint16_t reference_resistance;
  /**
   * @brief Supply voltage of the voltage divider [mV].
   * 
   * Valid range: ::sencty_kMinTemperatureSupplyVoltage <= value <= ::sencty_kMaxTemperatureSupplyVoltage.
   */
  uint16_t supply_voltage;
  /**
   * @brief Resistance of the resistor at 0°C [ohm].
   */
  uint16_t resistance;
  /**
   * @brief Temperature coefficient of the resistor [mOhm/°C].
   */
  uint16_t temperature_coefficient;
} sencty_TemperatureSensorConfiguration;

/**
 * @brief Struct for the configuration of all sensors.
 */
typedef struct {
  /**
   * @brief Configuration of a temperature sensor.
   */
  sencty_TemperatureSensorConfiguration temperature_sensor;
} sencty_SensorConfiguration;

/**
 * @brief Minimum GPIO pin of a temperature sensor.
 */
extern const uint8_t sencty_kMinTemperatureADCChannel;

/**
 * @brief Maximum GPIO pin of a temperature sensor.
 */
extern const uint8_t sencty_kMaxTemperatureADCChannel;

/**
 * @brief Minimum supply voltage of the voltage divider [mV].
 */
extern const uint16_t sencty_kMinTemperatureSupplyVoltage;

/**
 * @brief Maximum supply voltage of the voltage divider [mV].
 */
extern const uint16_t sencty_kMaxTemperatureSupplyVoltage;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENCTY_SENSOR_CONFIG_TYPES_H_
