/**
 * @file sentem_sensor_temperature.c
 *
 * @brief Implementation of the the sensor temperature module.
 */
#include "sentem_sensor_temperature.h"

#include <stdbool.h>
#include <stdint.h>

#include "common/comass_common_assert.h"
#include "common/comdef_common_definitions.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "hal/adc_types.h"
#include "sensor/sencty_sensor_config_types.h"

#define MILLIVOLTS_PER_VOLT (1000.0F)      ///< Millivolts per volt [mV].
#define MILLIOHMS_PER_OHM (1000.0F)        ///< Milliohms per ohm [ohm].
#define ADC_BITWIDTH (12U)                 ///< Bitwidth of the ADC [bits].
#define ADC_RESOLUTION (2U ^ ADC_BITWIDTH) ///< Resolution of the ADC [steps].

/**
 * @brief Initialization state of the module.
 */
static bool sentem_initialized = false;

/**
 * @brief Temperature sensor configuration.
 */
static sencty_TemperatureSensorConfiguration sentem_configuration = {0};

/**
 * @brief ADC unit handle.
 */
static adc_oneshot_unit_handle_t sentem_adc_handle = NULL;

void sentem_Init(const sencty_TemperatureSensorConfiguration configuration) {
  comass_AssertTrue(!sentem_initialized, comdef_kAlreadyInitialized);
  comass_AssertU8InRange((uint8_t)configuration.adc_channel, sencty_kMinTemperatureADCChannel,
                         sencty_kMaxTemperatureADCChannel, comdef_kInvalidParameter);

  const adc_oneshot_unit_init_cfg_t kADCInitConfig = {
      .unit_id = ADC_UNIT_1,
  };
  comass_AssertTrue(ESP_OK == adc_oneshot_new_unit(&kADCInitConfig, &sentem_adc_handle), comdef_kInternalError);

  const adc_oneshot_chan_cfg_t kADCChannelConfig = {
      .bitwidth = ADC_BITWIDTH,
      .atten = ADC_ATTEN_DB_12,
  };
  comass_AssertTrue(ESP_OK ==
                        adc_oneshot_config_channel(sentem_adc_handle, configuration.adc_channel, &kADCChannelConfig),
                    comdef_kInternalError);

  sentem_configuration = configuration;
  sentem_initialized = true;
}

int8_t sentem_ReadTemperature(void) {
  comass_AssertTrue(sentem_initialized, comdef_kNotInitialized);
  comass_AssertNotNull(sentem_adc_handle, comdef_kInternalError);

  int8_t temperature = INT8_MIN;

  int raw_value = 0;
  const esp_err_t kReadResult = adc_oneshot_read(sentem_adc_handle, sentem_configuration.adc_channel, &raw_value);
  if (kReadResult == ESP_OK) {
    const float kADCVoltage =
        ((float)raw_value / ADC_RESOLUTION) * ((float)sentem_configuration.supply_voltage / MILLIVOLTS_PER_VOLT);

    const float kResistancePT =
        (float)sentem_configuration.reference_resistance *
        (kADCVoltage / (((float)sentem_configuration.supply_voltage / MILLIVOLTS_PER_VOLT) - kADCVoltage));

    const float kTemperature = (kResistancePT - (float)sentem_configuration.resistance) /
                               ((float)sentem_configuration.temperature_coefficient / MILLIOHMS_PER_OHM);

    if ((kTemperature >= (float)INT8_MIN) && (kTemperature <= (float)INT8_MAX)) {
      temperature = (int8_t)kTemperature;
    }
  }

  return temperature;
}
