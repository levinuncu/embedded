/**
 * @brief Implementation of the the sensor gnss module.
 */
#include "sengns_sensor_gnss.h"

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "driver/uart.h"
#include "hal/uart_types.h"
#include "portmacro.h"
#include "esp_err.h"
#include "esp_log.h"

#include "sensor/senaty_sensor_api_types.h"
#include "sensor/sencty_sensor_config_types.h"

#define UART_BUFFER_SIZE (1024)
#define UART_TIMEOUT_MS (20)
#define COORDINATE_SCALE (10000.0)

#define INVALID_POSITION (UINT32_MAX) //< Value for an invalid position.
#define INVALID_TIMESTAMP (UINT32_MAX) //< Value for an invalid timestamp.

typedef struct {
	double latitude_raw;
	char latitude_direction;
	double longitude_raw;
	char longitude_direction;
} GnrmcFields;

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "SENGNS";

/**
 * @brief Initialization state of the module.
 */
static bool initialized = false;

/**
 * @brief Configuration of the sensor.
 */
static sencty_GnssSensorConfiguration configuration;

static bool ParseGNRMC(const char *const nmea_strings, GnrmcFields *const fields);
static bool EncodeCoordinate(double raw_coordinate, char direction, uint32_t *const data);

void sengns_Init(const sencty_GnssSensorConfiguration sensor_configuration) {
  configuration = sensor_configuration;

  const uart_config_t kUartConfig = {
			.baud_rate = configuration.uart_baud_rate_hz,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.source_clk = UART_SCLK_DEFAULT,
  };

  const esp_err_t kDriverInstallResult = uart_driver_install(configuration.uart_port, UART_BUFFER_SIZE * 2, 0, 0, NULL, 0);
  if (kDriverInstallResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to install the UART driver: %s", esp_err_to_name(kDriverInstallResult));
    return;
  }

  const esp_err_t kConfigResult = uart_param_config(configuration.uart_port, &kUartConfig);
  if (kConfigResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to configure the UART: %s", esp_err_to_name(kConfigResult));
    return;
  }

  const esp_err_t kSetLevelResult = uart_set_pin(configuration.uart_port, configuration.uart_tx_gpio, configuration.uart_rx_gpio, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  if (kConfigResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to assign the UART pins: %s", esp_err_to_name(kSetLevelResult));
    return;
  }

  ESP_LOGI(kLoggerTag, "Initialized");
  initialized = true;
}

senaty_GnssSensorReading sengns_ReadData(void) {
  const senaty_GnssSensorReading kFailedReading = {
    .longitude = INVALID_POSITION,
    .latitude = INVALID_POSITION,
		.timestamp = INVALID_TIMESTAMP,
  };

  if (!initialized) {
    return kFailedReading;
  }

  uint8_t nmea_strings[UART_BUFFER_SIZE] = {0};
	const int bytes_read = uart_read_bytes(configuration.uart_port, nmea_strings, UART_BUFFER_SIZE - 1, UART_TIMEOUT_MS / portTICK_PERIOD_MS);
	if (bytes_read <= 0) {
		ESP_LOGE(kLoggerTag, "Sensor send no data");
		return kFailedReading;
	}
  
  nmea_strings[bytes_read] = '\0';

	GnrmcFields fields = {0};
	if (!ParseGNRMC((const char *)nmea_strings, &fields)) {
		ESP_LOGE(kLoggerTag, "Parsing of data failed");
		return kFailedReading;
	}

  senaty_GnssSensorReading reading = {0};
  if (!EncodeCoordinate(fields.longitude_raw, fields.longitude_direction, &reading.longitude)) {
		return kFailedReading;
  }

  if (!EncodeCoordinate(fields.latitude_raw, fields.latitude_direction, &reading.latitude)) {
		return kFailedReading;
  }

	// TODO: Get the real value from the satellite
	reading.timestamp = 123456;

  ESP_LOGI(kLoggerTag, "Read sensor data");
  return reading;
}

static bool ParseGNRMC(const char *const nmea_strings, GnrmcFields *const fields) {
	const char *start = strstr(nmea_strings, "$GNRMC,");
	if (start == NULL) {
		return false;
	}

	const char *end = strstr(start, "\r\n");
	const size_t line_length = (end != NULL) ? (size_t)(end - start) : strlen(start);
	if ((line_length == 0U) || (line_length >= 127U)) {
		return false;
	}

	char sentence[128] = {0};
	memcpy(sentence, start, line_length);
	sentence[line_length] = '\0';

	int token_index = 0;
	char *token = strtok(sentence, ",");
	while (token != NULL) {
		switch (token_index) {
			case 2:
				fields->latitude_raw = strtod(token, NULL);
				break;
			case 3:
				fields->latitude_direction = token[0];
				break;
			case 4:
				fields->longitude_raw = strtod(token, NULL);
				break;
			case 5:
				fields->longitude_direction = token[0];
				break;
			default:
				break;
		}

		++token_index;
		token = strtok(NULL, ",");
	}

	if ((fields->latitude_direction != 'N') && (fields->latitude_direction != 'S')) {
		return false;
	}

	if ((fields->longitude_direction != 'E') && (fields->longitude_direction != 'W')) {
		return false;
	}

	return true;
}

static bool EncodeCoordinate(double raw_coordinate, char direction, uint32_t *const data) {
	if (raw_coordinate <= 0.0) {
		ESP_LOGW(kLoggerTag, "Raw coordinate smaller than 0");
		return false;
	}

	const double degrees_component = floor(raw_coordinate / 100.0);
	const double minutes_component = raw_coordinate - (degrees_component * 100.0);
	const double decimal_degree = degrees_component + (minutes_component / 60.0);
	const double scaled_coordinate = decimal_degree * COORDINATE_SCALE;

	if (scaled_coordinate < 0.0) {
		ESP_LOGW(kLoggerTag, "Scaled coordinate smaller than 0");
		return false;
	}

	uint32_t encoded = (uint32_t)(scaled_coordinate + 0.5);
	if ((direction == 'S') || (direction == 'W')) {
		encoded |= 0x80000000U;
	}

  *data = encoded;
	return true;
}
