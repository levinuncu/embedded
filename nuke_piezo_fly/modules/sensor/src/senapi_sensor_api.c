/**
 * @brief Implementation of the the sensor api module.
 */
#include "sensor/senapi_sensor_api.h"

#include "sengns_sensor_gnss.h"
#include "senimu_sensor_imu.h"
#include "sentem_sensor_temperature.h"

void senapi_Init(const sencty_SensorsConfiguration sensors_configuration) {
  sengns_Init(sensors_configuration.gnss_sensor);
  senimu_Init(sensors_configuration.imu_sensor);
  sentem_Init(sensors_configuration.temperature_sensor);
}

void senapi_Deinit(void) {
  senimu_Deinit();
}

senaty_SensorsReading senapi_ReadData(void) {
  senaty_SensorsReading readings = {
    .gnss_sensor = sengns_ReadData(),
    .imu_sensor = senimu_ReadData(),
    .temperature_sensor = sentem_ReadData(),
  };

  return readings;
}
