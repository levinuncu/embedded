#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "ext_gnss.h"

static const char *TAG = "GNSS";

/* Initialisiert den UART-Treiber für die GNSS-Kommunikation */
esp_err_t gnss_uart_init(int tx_pin, int rx_pin){
    /* Parameter für den UART-Treiber konfigurieren */
    uart_config_t uart_config = {       // Insgesamt werden 10 Bits pro Zeichen übertragen (1 Startbit, 8 Datenbits, 1 Stoppbit)
        .baud_rate = UART_BAUD_RATE,        // Baudrate für das GNSS-Module
        .data_bits = UART_DATA_8_BITS,      // 8 Datenbits (Größe eines Zeichens)
        .parity    = UART_PARITY_DISABLE,   // Keine Parität (Fehlererkennung deaktiviert)
        .stop_bits = UART_STOP_BITS_1,      // 1 Stoppbit (Ende eines Zeichens)
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  // Keine Hardware-Flusskontrolle (CTS/RTS, stop/weitersenden deaktiviert)
        .source_clk = UART_SCLK_DEFAULT,    // Standard-UART-Clockquelle
    };

    int intr_alloc_flags = 0;

    #if CONFIG_UART_ISR_IN_IRAM
        intr_alloc_flags = ESP_INTR_FLAG_IRAM;
    #endif

    /* Installieren des UART-Treibers und Senden der Konfiguration bzw Setzen der Pins */
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, tx_pin, rx_pin, PIN_RTS, PIN_CTS));

    return ESP_OK;
}

/* Liest die GNSS-Messwerte aus und parst die Informationen aus dem NMEA-String (hier $GPRMC) */
bool gnss_read_measurement(gnss_measurement_t *measurement){
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    int len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
    if (len <= 0) {
        free(data);
        return false;
    }
    data[len] = '\0';

    /* Datenverarbeitung, Parsen der NMEA-Strings */
    if (parse_gprmc_sentence((char *)data, measurement) == ESP_OK) {
        free(data);
        return true;
    } else {
        ESP_LOGW(TAG, "GPRMC sentence not found");
    }

    free(data);
    return false;
}

/* Extrahiert die $GPRMC-Nachricht aus den NMEA-Strings */
esp_err_t get_gprmc_sentence(const char *nmea_strings, char *sentence) {
    /* Sucht nach der $GPRMC-Nachricht */
    const char *start = strstr(nmea_strings, "$GPRMC,");
    if (start == NULL) {
        return ESP_ERR_NOT_FOUND;
    }

    /* Prüft, ob die Prüfsumme gültig ist */
    const char *chksm = strchr(start, '*');
    if (chksm == NULL || chksm[1] == '\0' || chksm[2] == '\0') {
        return ESP_ERR_INVALID_RESPONSE;
    }

    /* Extrahiert die Nachricht ohne Prüfsumme */
    const char *payload_start = start + strlen("$GPRMC,");
    size_t payload_len = (size_t)(chksm - payload_start);
    if (payload_len + 4 > 128) {
        return ESP_ERR_NO_MEM;
    }

    /* Kopiert die Nachricht ohne Prüfsumme in den Zielbuffer */
    memcpy(sentence, payload_start, payload_len);
    sentence[payload_len] = '\0';


    return ESP_OK;
}

/* Parst die $GPRMC-Nachricht und extrahiert die Messwerte */
esp_err_t parse_gprmc_sentence(const char *nmea_strings, gnss_measurement_t *measurement) {
    /* Extrahiert die $GPRMC-Nachricht */
    char sentence[128];
    if (get_gprmc_sentence(nmea_strings, sentence) != ESP_OK) {
        return ESP_ERR_NOT_FOUND;
    }

    memset(measurement, 0, sizeof(*measurement));

    /* Parst die Tokens der NMEA-Nachricht und schreibt sie in das GNSS Measurement */
    int counter = 0;
    char *token = strtok(sentence, ",");
    while (token != NULL) {
        counter++;
        switch (counter) {
            case 1: // Zeit
                if (strlen(token) >= 6) {
                    measurement->datetime.hours = (uint8_t)((token[0] - '0') * 10 + (token[1] - '0'));
                    measurement->datetime.minutes = (uint8_t)((token[2] - '0') * 10 + (token[3] - '0'));
                    measurement->datetime.seconds = (uint8_t)((token[4] - '0') * 10 + (token[5] - '0'));
                }
                break;
            case 2: // Status
                measurement->status = token[0];
                break;
            case 3: // Latitude
                measurement->position.lat = (int32_t)strtod(token, NULL);
                break;
            case 4: // N/S
                measurement->position.lat_dir = token[0];
                break;
            case 5: // Longitude
                measurement->position.lon = (int32_t)strtod(token, NULL);
                break;
            case 6: // E/W
                measurement->position.lon_dir = token[0];
                break;
            case 7: // Geschwindigkeit über Grund (Knoten)
                measurement->speed = (int32_t)strtod(token, NULL);
                break;
            case 8: // Kurs über Grund (Grad)
                measurement->course = (int32_t)strtod(token, NULL);
                break;
            case 9: // Datum
                if (strlen(token) >= 6) {
                    measurement->datetime.day = (uint8_t)((token[0] - '0') * 10 + (token[1] - '0'));
                    measurement->datetime.month = (uint8_t)((token[2] - '0') * 10 + (token[3] - '0'));
                    measurement->datetime.year = (uint16_t)(2000 + (token[4] - '0') * 10 + (token[5] - '0'));
                }
                break;
            default:
                break;
        }

        token = strtok(NULL, ",");
    }

    return ESP_OK;
}