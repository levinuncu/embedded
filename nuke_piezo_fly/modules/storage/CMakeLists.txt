idf_component_register(
    SRCS
        "src/stoapi_storage_api.c"

    INCLUDE_DIRS
        "include"

    PRIV_INCLUDE_DIRS
        "src"

    REQUIRES
        common nvs_flash
)