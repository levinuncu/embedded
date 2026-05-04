/**
 * @file stoctr_storage_control.h
 *
 * @addtogroup storage_control
 * @{
 *
 * @brief Interface of the storage control.
 * 
 * This module provides functions used to control the storage.
 */
#ifndef STOCTR_STORAGE_CONTROL_H_
#define STOCTR_STORAGE_CONTROL_H_

#include <stddef.h>

#include "storage/stocty_storage_config_types.h"
#include "sensor/senaty_sensor_api_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the storage control module.
 *
 * This function is used to initialize the control module. If the initialization fails, a fatal error is thrown.
 * This function is only a wrapper function, which calls stoapi_Init().
 * 
 * @param [in] storage_configuration Pointer to the storage configuration.
 */
void stoctr_Init(const stocty_StorageConfiguration *const storage_configuration);

/**
 * @brief Write data to the storage.
 * 
 * This functions writes data to the storage in the configured namespace. If the writing fails, a fatal error is thrown.
 * This function is only a wrapper function, which calls stoapi_WriteToStorage().
 * 
 * @param [in] sensors_readings Pointer to the sensors readings.
 * @param [in] number_of_sensors_readings Number of sensors readings.
 */
void stoctr_WriteToStorage(const senaty_SensorsReading *const sensors_readings, const size_t number_of_sensors_readings);

/**
 * @brief Read data from the storage.
 * 
 * This functions reads data from the storage in the configured namespace. If the reading fails or the external buffer is to small, a fatal error is thrown.
 * This function is only a wrapper function, which calls stoapi_ReadFromStorage().
 * 
 * @param [in] sensors_readings_size Size of the external sensors readings.
 * @param [out] sensors_readings Pointer to the sensors readings.
 * @param [out] number_of_sensors_readings Number of read sensors readings.
 */
void stoctr_ReadFromStorage(const size_t sensors_readings_size, senaty_SensorsReading *const sensors_readings, size_t *const number_of_sensors_readings);

/**
 * @brief Clear the storage.
 * 
 * This functions clears the whole storage in the configured namespace. If the clearing fails, a fatal error is thrown.
 * This function is only a wrapper function, which calls stoapi_ClearStorage().
 */
void stoctr_ClearStorage(void);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // STOCTR_STORAGE_CONTROL_H_
