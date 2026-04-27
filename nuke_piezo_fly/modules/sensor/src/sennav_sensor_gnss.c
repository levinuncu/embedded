/**
 * @file sennav_sensor_gnss.c
 *
 * @brief Implementation of the sensor GNSS module.
 */

#include "sennav_sensor_gnss.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "common/comass_common_assert.h"
#include "common/comdef_common_definitions.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

#define SENNAV_UART_BUFFER_SIZE (1024)
#define SENNAV_UART_TIMEOUT_MS (20)
#define SENNAV_COORDINATE_SCALE (10000.0)

typedef struct {
	double latitude_raw;
	char latitude_direction;
	double longitude_raw;
	char longitude_direction;
} sennav_GnrmcFields;

static bool sennav_initialized = false;
static uart_port_t sennav_uart_port = UART_NUM_1;

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "GNSS";

static bool ParseGNRMC(const char *const nmea_strings, sennav_GnrmcFields *const fields);
static uint32_t EncodeCoordinate(double raw_coordinate, char direction);

void sennav_Init(const sencty_GNSSSensorConfiguration configuration) {
	comass_AssertTrue(!sennav_initialized, comdef_kAlreadyInitialized);
	comass_AssertTrue(GPIO_IS_VALID_GPIO((gpio_num_t)configuration.uart_tx_gpio), comdef_kInvalidParameter);
	comass_AssertTrue(GPIO_IS_VALID_GPIO((gpio_num_t)configuration.uart_rx_gpio), comdef_kInvalidParameter);
	comass_AssertTrue(configuration.uart_baud_rate_hz > 0U, comdef_kInvalidParameter);

	sennav_uart_port = (uart_port_t)configuration.uart_port;

	const uart_config_t kUartConfiguration = {
			.baud_rate = (int)configuration.uart_baud_rate_hz,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.source_clk = UART_SCLK_DEFAULT,
	};

	comass_AssertTrue(ESP_OK == uart_driver_install(sennav_uart_port, SENNAV_UART_BUFFER_SIZE * 2, 0, 0, NULL, 0),
										comdef_kInternalError);
	comass_AssertTrue(ESP_OK == uart_param_config(sennav_uart_port, &kUartConfiguration), comdef_kInternalError);
	comass_AssertTrue(
			ESP_OK == uart_set_pin(sennav_uart_port, (int)configuration.uart_tx_gpio, (int)configuration.uart_rx_gpio,
														 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE),
			comdef_kInternalError);


  ESP_LOGI(kLoggerTag, "GNSS initialized on UART port TX: %u, RX: %u", configuration.uart_tx_gpio, configuration.uart_tx_gpio);

	sennav_initialized = true;
}

bool sennav_ReadData(sennav_GnssData *const reading) {
	comass_AssertTrue(sennav_initialized, comdef_kNotInitialized);
	comass_AssertNotNull(reading, comdef_kInvalidParameter);

	reading->longitude = SENNAV_INVALID_COORDINATE;
	reading->latitude = SENNAV_INVALID_COORDINATE;

	uint8_t nmea_strings[SENNAV_UART_BUFFER_SIZE] = {0};
	const int bytes_read = uart_read_bytes(sennav_uart_port, nmea_strings, SENNAV_UART_BUFFER_SIZE - 1,
																				 SENNAV_UART_TIMEOUT_MS / portTICK_PERIOD_MS);
	if (bytes_read <= 0) {
		ESP_LOGW(kLoggerTag, "No data received");
		return false;
	}

	nmea_strings[bytes_read] = '\0';

	sennav_GnrmcFields fields = {0};
	if (!ParseGNRMC((const char *)nmea_strings, &fields)) {
		ESP_LOGW(kLoggerTag, "Parsing of data failed");
		return false;
	}

	reading->latitude = EncodeCoordinate(fields.latitude_raw, fields.latitude_direction);
	reading->longitude = EncodeCoordinate(fields.longitude_raw, fields.longitude_direction);

	return (reading->latitude != SENNAV_INVALID_COORDINATE) && (reading->longitude != SENNAV_INVALID_COORDINATE);
}

static bool ParseGNRMC(const char *const nmea_strings, sennav_GnrmcFields *const fields) {
	comass_AssertNotNull(nmea_strings, comdef_kInvalidParameter);
	comass_AssertNotNull(fields, comdef_kInvalidParameter);

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

static uint32_t EncodeCoordinate(double raw_coordinate, char direction) {
	if (raw_coordinate <= 0.0) {
		ESP_LOGW(kLoggerTag, "Raw coordinate smaller than 0");
		return SENNAV_INVALID_COORDINATE;
	}

	const double degrees_component = floor(raw_coordinate / 100.0);
	const double minutes_component = raw_coordinate - (degrees_component * 100.0);
	const double decimal_degree = degrees_component + (minutes_component / 60.0);
	const double scaled_coordinate = decimal_degree * SENNAV_COORDINATE_SCALE;

	if (scaled_coordinate < 0.0) {
		ESP_LOGW(kLoggerTag, "Scaled coordinate smaller than 0");
		return SENNAV_INVALID_COORDINATE;
	}

	uint32_t encoded = (uint32_t)(scaled_coordinate + 0.5);
	if ((direction == 'S') || (direction == 'W')) {
		encoded |= 0x80000000U;
	}

	return encoded;
}
