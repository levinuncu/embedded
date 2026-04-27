/**
 * @file sennav_sensor_gnss.h
 *
 * @brief Interface of the sensor GNSS module.
 */
#ifndef SENNAV_SENSOR_GNSS_H_
#define SENNAV_SENSOR_GNSS_H_

#include <stdbool.h>
#include <stdint.h>

#include "sensor/sencty_sensor_config_types.h"

typedef struct {
	uint32_t longitude;
	uint32_t latitude;
} sennav_GnssData;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void sennav_Init(const sencty_GNSSSensorConfiguration configuration);
bool sennav_ReadData(sennav_GnssData *const reading);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SENNAV_SENSOR_GNSS_H_
