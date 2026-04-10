idf_component_register(
    SRCS "ext_mpu6050.c" "ext_i2c.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_i2c esp_driver_gpio mpu6050
)