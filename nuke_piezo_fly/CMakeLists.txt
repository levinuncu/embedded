cmake_minimum_required(VERSION 3.16.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND EXTRA_COMPONENT_DIRS modules)
list(APPEND EXTRA_COMPONENT_DIRS components)

if(CMAKE_HOST_WIN32)
  include("$ENV{USERPROFILE}/.platformio/packages/framework-espidf/tools/cmake/project.cmake")
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

