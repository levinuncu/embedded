idf_component_register(
    SRCS "ble.c" "gatt_service.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash console sensor
)
