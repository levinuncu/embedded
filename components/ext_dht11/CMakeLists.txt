idf_component_register(
    SRCS "dht11.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES freertos esp_driver_gpio esp_timer log esp_rom
)