#pragma once

#include <stdint.h>

/* Hilfsstrukturen für die GNSS-Daten */
typedef struct {
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} packet_date_time_t;

typedef struct {
    int32_t lat;
    char lat_dir;
    int32_t lon;
    char lon_dir;
} packet_gnss_position_t;

typedef struct {
    char status;
    packet_date_time_t datetime;
    packet_gnss_position_t position;
    int32_t speed;
    int32_t course;
} packet_gnss_measurement_t;

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
    packet_gnss_measurement_t gnss;
};