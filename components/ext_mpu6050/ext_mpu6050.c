#include "ext_mpu6050.h"
#include "ext_i2c.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "mpu6050.h"

static const char *TAG = "MPU6050";

static i2c_master_bus_handle_t s_bus = NULL;
static i2c_master_dev_handle_t s_dev = NULL;
static bool s_ready = false;

#define MPU6050_REG_PWR_MGMT_1  0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_WHO_AM_I    0x75

static esp_err_t mpu_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_dev, buf, sizeof(buf), -1);
}

static esp_err_t mpu_read_regs(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(s_dev, &reg, 1, data, len, -1);
}

void mpu6050_init(int sda_pin, int scl_pin)
{
    i2c_init(sda_pin, scl_pin, &s_bus, &s_dev);
    if (s_dev == NULL) {
        ESP_LOGE(TAG, "I2C init failed");
        return;
    }

    uint8_t who = 0;
    esp_err_t ret = mpu_read_regs(MPU6050_REG_WHO_AM_I, &who, 1);
    if (ret != ESP_OK || who != 0x68) {
        ESP_LOGE(TAG, "MPU6050 not found, WHO_AM_I read returned: 0x%02X (err: %d)", who, ret);
        return;
    }
    ESP_LOGI(TAG, "MPU6050 detected at I2C address 0x68");

    ret = mpu_write_reg(MPU6050_REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake MPU6050 (err: %d)", ret);
        return;
    }

    s_ready = true;
    ESP_LOGI(TAG, "MPU6050 initialized");
}

bool mpu6050_read(mpu6050_measurement_t *m)
{
    if (!s_ready || m == NULL) {
        if (m == NULL) ESP_LOGE(TAG, "mpu6050_read: measurement pointer is NULL");
        else ESP_LOGE(TAG, "mpu6050_read: MPU6050 not ready");
        return false;
    }

    uint8_t raw[14];
    esp_err_t ret = mpu_read_regs(MPU6050_REG_ACCEL_XOUT_H, raw, sizeof(raw));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "mpu6050_read: I2C read failed (err: %d)", ret);
        return false;
    }

    m->x_accel = (int16_t)((raw[0] << 8) | raw[1]);
    m->y_accel = (int16_t)((raw[2] << 8) | raw[3]);
    m->z_accel = (int16_t)((raw[4] << 8) | raw[5]);

    m->x_gyro  = (int16_t)((raw[8] << 8) | raw[9]);
    m->y_gyro  = (int16_t)((raw[10] << 8) | raw[11]);
    m->z_gyro  = (int16_t)((raw[12] << 8) | raw[13]);

    return true;
}