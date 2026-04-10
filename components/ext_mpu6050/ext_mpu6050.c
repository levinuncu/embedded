#include "ext_mpu6050.h"
#include "ext_i2c.h"
#include "esp_log.h"
#include "mpu6050.h"

static const char *TAG = "MPU6050";

static mpu6050_handle_t sensor_handle = NULL;
static bool s_ready = false;

#define MPU6050_REG_PWR_MGMT_1  0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_WHO_AM_I    0x75

void mpu6050_init(int sda_pin, int scl_pin)
{
    if (i2c_init(sda_pin, scl_pin) != ESP_OK) {
        ESP_LOGE(TAG, "I2C init failed");
        s_ready = false;
        return;
    }

    sensor_handle = mpu6050_create(I2C_NUM_0, 0x68);
    if (sensor_handle == NULL) {
        ESP_LOGE(TAG, "mpu6050_create failed");
        s_ready = false;
        return;
    }

    if (mpu6050_wake_up(sensor_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up MPU6050");
        s_ready = false;
        return;
    }

    s_ready = true;
    ESP_LOGI(TAG, "MPU6050 initialized successfully");
}

bool mpu6050_read(mpu6050_measurement_t *m)
{
    if (!s_ready || m == NULL) {
        ESP_LOGE(TAG, "MPU6050 not ready or invalid measurement pointer");
        return false;
    }
    
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