#include "ext_mpu6050.h"
#include "esp_log.h"
#include "mpu6050.h"

static const char *TAG = "MPU6050";

static i2c_master_dev_handle_t s_dev = NULL;
static mpu6050_handle_t sensor_handle = NULL;
static bool s_ready = false;

/* Initialisiert den MPU6050 mit einem gegebenen I2C-Gerätehandle und weckt ihn auf */
void mpu6050_init_with_device(i2c_master_dev_handle_t dev_handle)
{
    if (dev_handle == NULL) {
        ESP_LOGE(TAG, "Invalid I2C device handle");
        return;
    }

    s_dev = dev_handle;
    sensor_handle = mpu6050_create(s_dev, 0x68);
    if (sensor_handle == NULL) {
        ESP_LOGE(TAG, "mpu6050_create failed");
        return;
    }

    if (mpu6050_wake_up(sensor_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up MPU6050");
        return;
    }

    mpu6050_config(sensor_handle, ACCE_FS_2G, GYRO_FS_250DPS);

    /* Setzt die ready-Flag des Programms für den MPU6050 */
    s_ready = true;
    ESP_LOGI(TAG, "MPU6050 initialized successfully");
}

/* Liest die Messwerte (Gyroskop und Beschleunigung) aus dem MPU6050 aus */
bool mpu6050_read(mpu6050_measurement_t *m)
{
    if (!s_ready || m == NULL) {
        ESP_LOGE(TAG, "MPU6050 not ready or invalid measurement pointer");
        return false;
    }
    
    /* Nutzt die MPU6050-Bibliothek, um die Sensordaten auszulesen */
    mpu6050_acce_value_t acce = {0};
    mpu6050_gyro_value_t gyro = {0};

    if (mpu6050_get_acce(sensor_handle, &acce) != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_get_acce failed");
        return false;
    }

    if (mpu6050_get_gyro(sensor_handle, &gyro) != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_get_gyro failed");
        return false;
    }

    m->x_accel = acce.acce_x;
    m->y_accel = acce.acce_y;
    m->z_accel = acce.acce_z;

    m->x_gyro = gyro.gyro_x;
    m->y_gyro = gyro.gyro_y;
    m->z_gyro = gyro.gyro_z;

    return true;

}