idf_component_register(
    SRCS
        "src/stoapi_storage_api.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        nvs_flash
)