idf_component_register(
    SRCS
        "src/senapi_sensor_api.c"
        "src/sencty_sensor_config_types.c"
        "src/sentem_sensor_temperature.c"
        "src/senimu_sensor_imu.c"
        "src/sennav_sensor_gnss.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "src"

    REQUIRES
        common freertos esp_driver_gpio esp_driver_i2c esp_driver_uart hal esp_timer log esp_rom mpu6050
)