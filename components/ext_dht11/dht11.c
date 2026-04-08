#include "dht11.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include <stdint.h>

static const char *TAG = "DHT11";
static int s_dht_pin = -1;

#define DHT11_RESPONSE_TIMEOUT_US 200
#define DHT11_BIT_START_TIMEOUT_US 100
#define DHT11_BIT_HIGH_TIMEOUT_US 100

/* Timer, der Timeouts für DHT11-Operationen verwaltet -> kein ewiges Polling */
static bool dht11_wait_for_level(int level, int timeout_us) {
    int64_t start = esp_timer_get_time();
    while (gpio_get_level(s_dht_pin) != level) {
        if ((esp_timer_get_time() - start) > timeout_us) {
            return false;
        }
    }
    return true;
}

// Initialisiert den Temperatursensor, indem er den Pin zuordnet und Datenrichtung und
// Standardsignal setzt.
void dht11_init(int pin) {
    s_dht_pin = pin;
    gpio_set_direction(s_dht_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_dht_pin, 1);
}

// Sendet das Startsignal an den DHT11, indem das Signal für 18ms auf LOW gesetzt wird.
static void dht11_start_signal(void) {
    gpio_set_direction(s_dht_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(s_dht_pin, 0);
    esp_rom_delay_us(18000);
    gpio_set_level(s_dht_pin, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(s_dht_pin, GPIO_MODE_INPUT);
}

// Liest ein Bit aus dem DHT11 aus. Ein HIGH-Signal über 40µs gilt als 1, darunter als 0.
static int dht11_read_bit(void) {
    if (!dht11_wait_for_level(1, DHT11_BIT_START_TIMEOUT_US)) {
        return -1;
    }

    int64_t start = esp_timer_get_time();
    if (!dht11_wait_for_level(0, DHT11_BIT_HIGH_TIMEOUT_US)) {
        return -1;
    }

    int64_t duration = esp_timer_get_time() - start;
    return (duration > 40) ? 1 : 0;
}

// Um ein Byte zu lesen, wird 8 Mal ein Bit gelesen.
static bool dht11_read_byte(int *byte_out) {
    int byte = 0;
    for (int i = 0; i < 8; i++) {
        int bit = dht11_read_bit();
        if (bit < 0) {
            // Fehler beim Lesen eines Bits z.B. Timeout, daher wird die Funktion mit false zurückgegeben
            return false;
        }
        byte <<= 1;
        byte |= bit;
    }

    *byte_out = byte;
    return true;
}

// Gibt false zurück, wenn: 
//      - der Funktionsparameter nicht valide ist
//      - der Pin nicht richtig zugewiesen wurde
//      - die Checksum der Daten nicht richtig ist (letzte 8 Bit der Summe der Daten)
// Sonst werden die Messdaten in der DHT11-Measurement Struktur zurückgegeben:
// Luftfeuchtigkeit (Ganzzahlteil und Dezimalteil), Temperatur (Ganzzahlteil und Dezimalteil)
bool dht11_read(dht11_measurement_t *reading) {
    if (reading == NULL || s_dht_pin < 0) {
        return false;
    }

    dht11_start_signal();

    // ESP wartet auf die Antwort des DHT11 in Form von LOW ("response signal"), HIGH ("preparation 
    // for sending data"), LOW ("50µs begin of data transmission")
    if (!dht11_wait_for_level(0, DHT11_RESPONSE_TIMEOUT_US) ||
        !dht11_wait_for_level(1, DHT11_RESPONSE_TIMEOUT_US) ||
        !dht11_wait_for_level(0, DHT11_RESPONSE_TIMEOUT_US)) {
        ESP_LOGW(TAG, "Sensor response timeout");
        return false;
    }

    // Lesen von 5 * 8 Bit für vollständigen Datensatz
    int checksum;
    if (!dht11_read_byte(&reading->humidity_int) ||
        !dht11_read_byte(&reading->humidity_dec) ||
        !dht11_read_byte(&reading->temperature_int) ||
        !dht11_read_byte(&reading->temperature_dec) ||
        !dht11_read_byte(&checksum)) {
        ESP_LOGW(TAG, "Timed out while reading sensor data");
        return false;
    }

    // Prüfung der Checksum, Error Log wenn inkorrekt
    int sum = reading->humidity_int + reading->humidity_dec + reading->temperature_int + reading->temperature_dec;
    if ((sum & 0xFF) != checksum) {
        ESP_LOGE(TAG, "Checksum error");
        return false;
    }

    return true;
}