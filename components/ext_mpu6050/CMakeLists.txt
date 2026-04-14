idf_component_register(
    SRCS "ext_mpu6050.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES "ext_i2c" "mpu6050"
)