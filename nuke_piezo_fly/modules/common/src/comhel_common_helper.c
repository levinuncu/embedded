/**
 * @file comhel_common_helper.c
 *
 * @brief Implementation of the helper functions.
 */
#include "common/comhel_common_helper.h"

#include <stdbool.h>
#include <stdint.h>

#include "common/comdef_common_definitions.h"
#include "common/comsys_common_system_adapter.h"

bool comhel_IsU8InRange(const uint8_t value, const uint8_t min_value, const uint8_t max_value) {
  if (min_value > max_value) {
    comsys_FatalError(comdef_kInvalidParameter);
  }

  bool value_in_range = true;

  if ((value < min_value) || (value > max_value)) {
    value_in_range = false;
  }

  return value_in_range;
}

bool comhel_IsU16InRange(const uint16_t value, const uint16_t min_value, const uint16_t max_value) {
  if (min_value > max_value) {
    comsys_FatalError(comdef_kInvalidParameter);
  }

  bool value_in_range = true;

  if ((value < min_value) || (value > max_value)) {
    value_in_range = false;
  }

  return value_in_range;
}

bool comhel_IsU32InRange(const uint32_t value, const uint32_t min_value, const uint32_t max_value) {
  if (min_value > max_value) {
    comsys_FatalError(comdef_kInvalidParameter);
  }

  bool value_in_range = true;

  if ((value < min_value) || (value > max_value)) {
    value_in_range = false;
  }

  return value_in_range;
}
