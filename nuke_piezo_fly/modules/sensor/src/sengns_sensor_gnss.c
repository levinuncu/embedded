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

	uint8_t buffer[UART_BUFFER_SIZE] = {0};
	int bytes_read = uart_read_bytes(
			configuration.uart_port,
			buffer,
			UART_BUFFER_SIZE - 1,
			UART_TIMEOUT_MS / portTICK_PERIOD_MS
	);
	if (bytes_read <= 0) {
		ESP_LOGE(kLoggerTag, "Sensor sent no data");
		return kFailedReading;
	}
  
  buffer[bytes_read] = '\0';

	GnrmcFields fields = {0};
	if (!ParseGNRMC((const char *)buffer, &fields)) {
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

static bool ParseGNRMC(const char *const nmea, GnrmcFields *const fields) {
	const char *start = strstr(nmea, "RMC,");
	if (!start) {
			ESP_LOGE(kLoggerTag, "No RMC sentence found");
			return false;
	}

	size_t len = strcspn(start, "\r\n");
	if (len == 0 || len >= 128) {
			ESP_LOGE(kLoggerTag, "Invalid sentence length");
			return false;
	}

	char sentence[128];
	memcpy(sentence, start, len);
	sentence[len] = '\0';

	char *saveptr = NULL;
	char *token = strtok_r(sentence, ",", &saveptr);

	int index = 0;

	double time_raw = NAN;
	int date_raw = -1;
	char status = 'V';

	double lat = NAN;
	double lon = NAN;
	char lat_dir = 0;
	char lon_dir = 0;

	while (token != NULL) {
		switch (index) {
			case 1: time_raw = strtod(token, NULL); break;
			case 2: status = token[0]; break;
			case 3: lat = strtod(token, NULL); break;
			case 4: lat_dir = token[0]; break;
			case 5: lon = strtod(token, NULL); break;
			case 6: lon_dir = token[0]; break;
			case 9: date_raw = atoi(token); break;
		}

		token = strtok_r(NULL, ",", &saveptr);
		index++;
	}

	if (status != 'A') {
			ESP_LOGW(kLoggerTag, "No valid GPS fix (status != A)");
			return false;
	}

	if (isnan(time_raw) || date_raw <= 0) {
			ESP_LOGE(kLoggerTag, "Invalid time/date");
			return false;
	}

	if (isnan(lat) || isnan(lon)) {
			ESP_LOGE(kLoggerTag, "Invalid position data");
			return false;
	}

	if (lat_dir != 'N' && lat_dir != 'S') {
			ESP_LOGE(kLoggerTag, "Invalid latitude direction");
			return false;
	}

	if (lon_dir != 'E' && lon_dir != 'W') {
			ESP_LOGE(kLoggerTag, "Invalid longitude direction");
			return false;
	}

	fields->latitude_raw = lat;
	fields->longitude_raw = lon;
	fields->latitude_direction = lat_dir;
	fields->longitude_direction = lon_dir;
	fields->timestamp = gnrmc_to_timestamp_ms(time_raw, date_raw);

	return true;
}

static bool EncodeCoordinate(double raw, char direction, uint32_t *const out) {
    if (!out || isnan(raw)) {
        return false;
    }

    double deg = floor(raw / 100.0);
    double min = raw - deg * 100.0;

    if (min < 0.0 || min >= 60.0) {
        ESP_LOGE(kLoggerTag, "Invalid NMEA minutes");
        return false;
    }

    double decimal = deg + (min / 60.0);
    double scaled = decimal * COORDINATE_SCALE;

    if (scaled < 0.0) {
        scaled = -scaled;
    }

    uint32_t encoded = (uint32_t)(scaled + 0.5);

    if (direction == 'S' || direction == 'W') {
        encoded |= 0x80000000U;
    }

    *out = encoded;
    return true;
}

uint64_t gnrmc_to_timestamp_ms(double time_raw, int date_raw)
{
    int hour   = (int)(time_raw / 10000);
    int minute = (int)((time_raw - hour * 10000) / 100);
    int second = (int)(time_raw - hour * 10000 - minute * 100);

    int millis = (int)((time_raw - (int)time_raw) * 1000.0 + 0.5);

    int day   = date_raw / 10000;
    int month = (date_raw / 100) % 100;
    int year  = date_raw % 100;

    struct tm t = {0};

    t.tm_year = (2000 + year) - 1900;
    t.tm_mon  = month - 1;
    t.tm_mday = day;

    t.tm_hour = hour;
    t.tm_min  = minute;
    t.tm_sec  = second;
    t.tm_isdst = 0;

    setenv("TZ", "UTC0", 1);
    tzset();

    time_t seconds = mktime(&t);
    if (seconds < 0) {
        return 0;
    }

    return ((uint64_t)seconds * 1000ULL) + millis;
}