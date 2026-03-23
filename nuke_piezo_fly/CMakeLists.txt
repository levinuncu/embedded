cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND EXTRA_COMPONENT_DIRS modules)

if(CMAKE_HOST_WIN32)
  # TODO: Check if this works on Windows. If not use: include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  # Which works most of the time but when using custom targets (format) it sometimes does not set the IDF_PATH env variable.
  include(%HOMEPATH%/.platformio/packages/framework-espidf/tools/cmake/project.cmake)
else()
  include(~/.platformio/packages/framework-espidf/tools/cmake/project.cmake)
endif()

include(cmake_helper/clang_format.cmake)

project(
  NukePiezoFly
  VERSION 0.1.0
  DESCRIPTION "Nuke Piezo Fly"
  LANGUAGES CXX C
)
