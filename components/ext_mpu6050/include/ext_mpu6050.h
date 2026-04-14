#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <driver/i2c_master.h>

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

void mpu6050_init_with_device(i2c_master_dev_handle_t dev_handle);
bool mpu6050_read(mpu6050_measurement_t *measurement);