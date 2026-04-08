#pragma once

#include <stdint.h>

/* Struktur der Daten aller Sensoren (inkl. Temperatur, Luftfeuchtigkeit, ) */
struct sensor_packet_t {
    float temperature;
    float humidity;
};