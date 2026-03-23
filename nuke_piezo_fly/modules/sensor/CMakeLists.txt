idf_component_register(
    SRCS
        "src/senapi_sensor_api.c"
        "src/sencty_sensor_config_types.c"
        "src/sentem_sensor_temperature.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "src"

    REQUIRES
        common esp_adc
)