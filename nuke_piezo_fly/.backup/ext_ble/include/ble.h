/**
 * @file ble.h
 *
 * @brief Interface of the BLE API.
 */
#ifndef BLEAPI_BLE_H_
#define BLEAPI_BLE_H_

#include "sensor/senaty_sensor_api_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the BLE API module.
 */
void bleapi_Init(void);

/**
 * @brief Send one sensor reading over BLE notification.
 *
 * @param [in] sensors_reading Pointer to the current sensor reading.
 */
void bleapi_SendSensorsReading(const senaty_SensorsReading *const sensors_reading);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // BLEAPI_BLE_H_