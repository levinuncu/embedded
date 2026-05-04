/**
 * @file comsys_system_adapter.c
 *
 * @brief Implementation of the system adapter functions.
 */

#include "common/comsys_common_system_adapter.h"

#include <stdbool.h>
#include <stdint.h>

#include "common/comdef_common_definitions.h"
#include "esp_log.h"

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "SystemAdapter";

void comsys_FatalError(const comdef_ReturnCode error_reason) {
  ESP_LOGE(kLoggerTag, "Fatal error occurred: %u", (uint32_t)error_reason);

  while (true) {
  }
}
