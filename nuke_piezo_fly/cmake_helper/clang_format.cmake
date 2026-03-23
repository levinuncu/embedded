file(GLOB_RECURSE PROJECT_FORMAT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.[ch]"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/**/*.[ch]"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.[ch]"
)

find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(CLANG_FORMAT_EXE)
  add_custom_target(format-fix
    COMMAND "${CLANG_FORMAT_EXE}" -i --style=file ${PROJECT_FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_custom_target(format-check
    COMMAND "${CLANG_FORMAT_EXE}" --dry-run --Werror --style=file ${PROJECT_FORMAT_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
else()
  message(STATUS "clang-format not found; format targets are disabled")
endif()
