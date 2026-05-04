/**
 * @file comass_common_assert.c
 *
 * @brief Implementation of the assert functions.
 */
#include "common/comass_common_assert.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/comdef_common_definitions.h"
#include "common/comhel_common_helper.h"
#include "common/comsys_common_system_adapter.h"

void comass_AssertNotNull(const void *const pointer, const comdef_ReturnCode error_reason) {
  if (!comhel_IsU32InRange((uint32_t)error_reason, (uint32_t)comdef_kMin, (uint32_t)comdef_kMax - 1U)) {
    comsys_FatalError(comdef_kInvalidParameter);
  }

  if (pointer == NULL) {
    comsys_FatalError(error_reason);
  }
}

void comass_AssertTrue(const bool condition, const comdef_ReturnCode error_reason) {
  if (!comhel_IsU32InRange((uint32_t)error_reason, (uint32_t)comdef_kMin, (uint32_t)comdef_kMax - 1U)) {
    comsys_FatalError(comdef_kInvalidParameter);
  }

  if (!condition) {
    comsys_FatalError(error_reason);
  }
}

void comass_AssertU8InRange(const uint8_t value, const uint8_t min_value, const uint8_t max_value,
                            const comdef_ReturnCode error_reason) {
  comass_AssertTrue(min_value <= max_value, comdef_kInvalidParameter);
  if (!comhel_IsU32InRange((uint32_t)error_reason, (uint32_t)comdef_kMin, (uint32_t)comdef_kMax - 1U)) {
    comsys_FatalError(comdef_kInvalidParameter);
  }

  if ((value < min_value) || (value > max_value)) {
    comsys_FatalError(error_reason);
  }
}
