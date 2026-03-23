/**
 * @file comhel_common_helper.h
 *
 * @addtogroup common_helper
 * @{
 *
 * @brief Interface of the helper functions.
 *
 * This module provides some helper functions for common used functionalities.
 */
#ifndef COMHEL_COMMON_HELPER_H_
#define COMHEL_COMMON_HELPER_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Checks if a uint8_t value is in a defined range.
 *
 * This helper function checks if a uint8_t value is inside a defined range. 
 * The minimum value must be smaller or equal to the maximum value, otherwise a ::comdef_kInvalidParameter fatal error is thrown.
 *
 * @param [in] value Value to check.
 * @param [in] min_value Minimum value for the check.
 * @param [in] max_value Maximum value for the check.
 * @return true -> value inside the range
 * @return false -> value outside the range
 */
bool comhel_IsU8InRange(const uint8_t value, const uint8_t min_value, const uint8_t max_value);

/**
 * @brief Checks if a uint16_t value is in a defined range.
 *
 * This helper function checks if a uint16_t value is inside a defined range. 
 * The minimum value must be smaller or equal to the maximum value, otherwise a ::comdef_kInvalidParameter fatal error is thrown.
 *
 * @param [in] value Value to check.
 * @param [in] min_value Minimum value for the check.
 * @param [in] max_value Maximum value for the check.
 * @return true -> value inside the range
 * @return false -> value outside the range
 */
bool comhel_IsU16InRange(const uint16_t value, const uint16_t min_value, const uint16_t max_value);

/**
 * @brief Checks if a uint32_t value is in a defined range.
 *
 * This helper function checks if a uint32_t value is inside a defined range. 
 * The minimum value must be smaller or equal to the maximum value, otherwise a ::comdef_kInvalidParameter fatal error is thrown.
 *
 * @param [in] value Value to check.
 * @param [in] min_value Minimum value for the check.
 * @param [in] max_value Maximum value for the check.
 * @return true -> value inside the range
 * @return false -> value outside the range
 */
bool comhel_IsU32InRange(const uint32_t value, const uint32_t min_value, const uint32_t max_value);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // COMHEL_COMMON_HELPER_H_
