/**
 * @file comass_common_assert.h
 *
 * @addtogroup common_assert
 * @{
 *
 * @brief Interface of the assert functions.
 *
 * This module provides all necessary assert functions to perform checks and throw a specific fatal error if the check fails.
 */
#ifndef COMASS_COMMON_ASSERT_H_
#define COMASS_COMMON_ASSERT_H_

#include <stdbool.h>
#include <stdint.h>

#include "common/comdef_common_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Assert if a pointer is not NULL.
 *
 * This function checks if a pointer is not NULL. If that is not the case, the passed error code is thrown as fatal error using ::comsys_FatalError.
 *
 * @param [in] pointer Pointer to check.
 * @param [in] error_reason Reason in case of error. Valid range: ::comdef_kMin <= value < ::comdef_kMax.
 */
void comass_AssertNotNull(const void *const pointer, const comdef_ReturnCode error_reason);

/**
 * @brief Assert a bool condition is true.
 *
 * This function checks if a bool condition is true. If that is not the case, the passed error code is thrown as fatal error using ::comsys_FatalError.
 *
 * @param [in] condition Condition to check.
 * @param [in] error_reason Reason in case of error. Valid range: ::comdef_kMin <= value < ::comdef_kMax.
 */
void comass_AssertTrue(const bool condition, const comdef_ReturnCode error_reason);

/**
 * @brief Assert a uint8_t value is in a defined range.
 *
 * This function checks if a uint8_t value is inside a defined range. If that is not the case, the passed error code is thrown as fatal error using ::comsys_FatalError.
 * The minimum value must be smaller or equal to the maximum value, otherwise a ::comdef_kInvalidParameter fatal error is thrown.
 *
 * @param [in] value Value to check.
 * @param [in] min_value Minimum value for the check.
 * @param [in] max_value Maximum value for the check.
 * @param [in] error_reason Reason in case of error. Valid range: ::comdef_kMin <= value < ::comdef_kMax.
 */
void comass_AssertU8InRange(const uint8_t value, const uint8_t min_value, const uint8_t max_value,
                            const comdef_ReturnCode error_reason);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // COMASS_COMMON_ASSERT_H_
