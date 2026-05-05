/**
 * @brief Interface of the bluetooth module.
 */
#ifndef BLEAPI_BLUETOOTH_API_H_
#define BLEAPI_BLUETOOTH_API_H_

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"

#include "sensor/senaty_sensor_api_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the bluetooth module.
 */
esp_err_t bleapi_Init(void);

/**
 * @brief Deinitialize the bluetooth module.
 */
void bleapi_Deinit(void);

/**
 * @brief Returns true if a client is connected.
 */
bool bleapi_IsClientConnected(void);

/**
 * @brief Returns true if a client has subscribed.
 */
bool bleapi_IsNotifyEnabled(void);

/**
 * @brief Send an array of sensors reading.
 */
void bleapi_SendSensorsReadings(const senaty_SensorsReading *const sensors_readings, const size_t number_of_readings);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // BLEAPI_BLUETOOTH_API_H_
