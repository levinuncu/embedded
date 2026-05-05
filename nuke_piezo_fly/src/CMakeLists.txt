idf_component_register(
  SRCS
    "${CMAKE_SOURCE_DIR}/src/main.c"
    "${CMAKE_SOURCE_DIR}/src/sencfg_sensor_config.c"
    "${CMAKE_SOURCE_DIR}/src/stocfg_storage_config.c"

  INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/include"
)
