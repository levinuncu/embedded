#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#define PIN_RTS (-1)
#define PIN_CTS (-1)

#define UART_PORT_NUM      (1)
#define UART_BAUD_RATE     (9600)
#define BUF_SIZE (1024)

/* NMEA-Strings kommen in folgender Strukur:
$GPVTG	// T Kurs, M Magnetischer Kurs, N Geschwindigkeit (Knoten), K Geschwindigkeit (km/h)
$GPGGA	// Zeit, Latitude, N/S, Longitude, E/W, GPS Quality, Anzahl der Satelliten in Benutzung, Höhe, Höheneinheit, Geoidtrennung, Geoidtrennungseinheit, Zeit seit letzter GPS-Aktualisierung, DGPS-Station-ID
$GPGSA	// Modus, Fix-Typ, IDs oder SVs der Satelliten in Benutzung, PDOP, HDOP, VDOP
$GPGSV	// GPS: Anzahl der Nachrichten, Nachrichtennummer, Anzahl der sichtbaren Satelliten, Informationen zu Satelliten (ID, Elevation, Azimuth, SNR)
$GLGSV	// GLONASS: Anzahl der Nachrichten, Nachrichtennummer, Anzahl der sichtbaren Satelliten, Informationen zu Satelliten (ID, Elevation, Azimuth, SNR)
$GPGLL	// Latitude, N/S, Longitude, E/W, Zeit, Status (A=aktiv, V=ungültig)
$GPTXT	// Textnachricht, Anzahl der Nachrichten, Nachrichtennummer, Text
$GPRMC/ $GNRMC  // Minimal Data: Zeit, Status (A=aktiv, V=ungültig), Latitude, N/S, Longitude, E/W, Geschwindigkeit über Grund (Knoten), Kurs über Grund (Grad), Datum, Magnetische Variation, E/W, Prüfsumme
*/

/* Zeitstruktur */
typedef struct {
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} date_time_t;

typedef struct {
	double lat;
	char lat_dir; // N/S
	double lon;
	char lon_dir; // E/W
} gnss_position_t;

/* GNSS-Datenstruktur */
typedef struct {
	char status; 			// A=aktiv, V=ungültig
	date_time_t datetime;
	gnss_position_t position;
	double speed; 			// Geschwindigkeit über Grund (Knoten)
	double course; 		// Kurs über Grund (Grad)
} gnss_measurement_t;

esp_err_t gnss_uart_init(int tx_pin, int rx_pin);
bool gnss_read_measurement(gnss_measurement_t *measurement);
esp_err_t get_gnrmc_sentence(const char *nmea_strings, char *sentence);
esp_err_t parse_gnrmc_sentence(const char *sentence, gnss_measurement_t *measurement);