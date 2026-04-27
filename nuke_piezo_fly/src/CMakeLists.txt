idf_component_register(
    SRCS
        "${CMAKE_SOURCE_DIR}/src/comsys_system_adapter.c"
        "${CMAKE_SOURCE_DIR}/src/main.c"
        "${CMAKE_SOURCE_DIR}/src/sencfg_sensor_config.c"
        "${CMAKE_SOURCE_DIR}/src/senctr_sensor_control.c"
        "${CMAKE_SOURCE_DIR}/src/stocfg_storage_config.c"
        "${CMAKE_SOURCE_DIR}/src/stoctr_storage_control.c"

    INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}/include"

    REQUIRES
        common sensor storage esp_driver_gpio
)