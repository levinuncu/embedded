/**
 * @file gatt_service.h
 *
 * @brief Interface of the BLE GATT service.
 */
#ifndef BLEGAT_GATT_SERVICE_H_
#define BLEGAT_GATT_SERVICE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Handle of the BLE sensor data characteristic.
 */
extern uint16_t blegat_sensor_data_handle;

/**
 * @brief Initialize BLE GATT services.
 *
 * @return 0 on success, non-zero on failure.
 */
int blegat_Init(void);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // BLEGAT_GATT_SERVICE_H_