/**
 * @file sencfg_sensor_config.h
 *
 * @addtogroup sensor_config
 * @{
 *
 * @brief Interface of the sensor configuration.
 * 
 * This module provides the configuration data structure for all sensors.
 */
#ifndef SENCFG_SENSOR_CONFIG_H_
#define SENCFG_SENSOR_CONFIG_H_

#include "sensor/sencty_sensor_config_types.h"

/**
 * @brief Configuration data of all sensors.
 */
extern const sencty_SensorConfiguration sencfg_sensor_configuration;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SENCFG_SENSOR_CONFIG_H_
