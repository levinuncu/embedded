/**
 * @file comdef_common_definitions.h
 *
 * @addtogroup common_definitions
 * @{
 *
 * @brief Common definitions.
 *
 * This module defines the common definitions, types and data structures used by the software.
 */
#ifndef COMDEF_COMMON_DEFINITIONS_H_
#define COMDEF_COMMON_DEFINITIONS_H_

/**
 * @brief Enum for function return codes.
 */
typedef enum {
  comdef_kMin = 0,                  ///< Min value for return code enum
  comdef_kNoError = 0,              ///< No error
  comdef_kNotInitialized = 1,       ///< Not initialized
  comdef_kAlreadyInitialized = 2,   ///< Already initialized
  comdef_kInvalidConfiguration = 3, ///< Invalid configuration
  comdef_kInvalidParameter = 4,     ///< Invalid parameter
  comdef_kInternalError = 5,        ///< Internal error
  comdef_kBufferTooSmall = 6,       ///< Buffer too small
  comdef_kMax,                      ///< Max value for return code enum
} comdef_ReturnCode;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // COMDEF_COMMON_DEFINITIONS_H_
