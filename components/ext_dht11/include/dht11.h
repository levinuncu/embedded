#pragma once

#include <stdbool.h>

// DHT11 Sensor Daten
// Liefert Luftfeuchtigkeit und Temperatur in einer Struktur
typedef struct {
    int humidity_int;
    int humidity_dec;
    int temperature_int;
    int temperature_dec;
} dht11_measurement_t;

// Initialisiert den DHT11 an dem angegebenen Pin
void dht11_init(int pin);

// Liest bei Aufruf einmalig die Sensordaten des DHT11
bool dht11_read(dht11_measurement_t *reading);