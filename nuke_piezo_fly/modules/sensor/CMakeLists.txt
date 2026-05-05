idf_component_register(
    SRCS
        "src/senapi_sensor_api.c"
        "src/sengns_sensor_gnss.c"
        "src/senimu_sensor_imu.c"
        "src/sentem_sensor_temperature.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "src"

    REQUIRES
        esp_driver_i2c 
    
    PRIV_REQUIRES
        esp_driver_gpio esp_driver_uart esp_timer mpu6050
)