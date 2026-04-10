#pragma once

#include <stdint.h>

/* Struktur der Daten aller Sensoren (inkl. Temperatur, Luftfeuchtigkeit, ) */
struct sensor_packet_t {
    float temperature;
    float humidity;
    int x_gyro;
    int y_gyro;
    int z_gyro;
    int x_accel;
    int y_accel;
    int z_accel;
};