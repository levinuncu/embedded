/**
 * @brief Implementation of the the sensor gnss module.
 */
#include "sengns_sensor_gnss.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

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
#define INVALID_TIMESTAMP (UINT64_MAX) //< Value for an invalid timestamp.

typedef struct {
	double latitude_raw;
	char latitude_direction;
	double longitude_raw;
	char longitude_direction;
	uint64_t timestamp;
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
uint64_t gnrmc_to_timestamp_ms(double time_raw, int date_raw);

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


	uint32_t longitude = INVALID_POSITION;
  if (!EncodeCoordinate(fields.longitude_raw, fields.longitude_direction, &longitude)) {
		return kFailedReading;
  }

	uint32_t latitude = INVALID_POSITION;
  if (!EncodeCoordinate(fields.latitude_raw, fields.latitude_direction, &latitude)) {
		return kFailedReading;
  }

	senaty_GnssSensorReading reading = {
		.longitude = longitude,
		.latitude = latitude,
		.timestamp = fields.timestamp,
	};

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

	fields->latitude_raw = -1;
	fields->longitude_raw = -1;

	double time_raw = -1;
	int date_raw = -1;
	char status = 'Z';
	int token_index = 0;
	char *token = strtok(sentence, ",");
	while (token != NULL) {
		switch (token_index) {
			case 0:
				time_raw = strtod(token, NULL);
				break;
			case 1:
				status = token[0];
				break;
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
			case 8:
				date_raw = atoi(token);
				break;
			default:
				break;
		}

		++token_index;
		token = strtok(NULL, ",");
	}

	if (status != 'A') {
		return false;
	}

	if ((date_raw == -1) || (time_raw == -1)) {
		return false;
	}

	if ((fields->latitude_raw == -1) || (fields->longitude_raw == -1)) {
		return false;
	}

	if ((fields->latitude_direction != 'N') && (fields->latitude_direction != 'S')) {
		return false;
	}

	if ((fields->longitude_direction != 'E') && (fields->longitude_direction != 'W')) {
		return false;
	}

	fields->timestamp = gnrmc_to_timestamp_ms(time_raw, date_raw);

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

uint64_t gnrmc_to_timestamp_ms(double time_raw, int date_raw)
{
    // --- Zeit zerlegen (hhmmss.sss) ---
    int hour   = (int)(time_raw / 10000);
    int minute = (int)((time_raw - hour * 10000) / 100);
    int second = (int)(time_raw - hour * 10000 - minute * 100);

    int millis = (int)((time_raw - (int)time_raw) * 1000.0 + 0.5);

    // --- Datum zerlegen (ddmmyy) ---
    int day   = date_raw / 10000;
    int month = (date_raw / 100) % 100;
    int year  = date_raw % 100;   // GPS: yy

    // --- struct tm füllen ---
    struct tm t = {0};
    t.tm_year = (2000 + year) - 1900;  // Jahre seit 1900
    t.tm_mon  = month - 1;             // 0–11
    t.tm_mday = day;

    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;

    t.tm_isdst = 0; // kein DST (UTC!)

    // --- WICHTIG: auf UTC setzen ---
    setenv("TZ", "UTC0", 1);
    tzset();

    // --- Sekunden seit Epoch ---
    time_t seconds = mktime(&t);
    if (seconds < 0) {
        return 0;
    }

    // --- Millisekunden ---
    uint64_t timestamp_ms =
        ((uint64_t)seconds * 1000ULL) +
        (uint64_t)millis;

    return timestamp_ms;
}