#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Eigene MPU6050 Schnittstelle für das Auslesen von Sensordaten
*   stützt sich intern auf I2C und die MPU6050-Bibliothek von Espressif 
*/

/* MPU6050 Sensor Datenstruktur */
typedef struct {
    int x_gyro;
    int y_gyro;
    int z_gyro;
    int x_accel;
    int y_accel;
    int z_accel;
} mpu6050_measurement_t;

void mpu6050_init(int sda_pin, int scl_pin);
bool mpu6050_read(mpu6050_measurement_t *measurement);