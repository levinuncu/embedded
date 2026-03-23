/**
 * @file comsys_common_system_adapter.h
 *
 * @addtogroup common_system_adapter
 * @{
 *
 * @brief Interface of the system adapter functions.
 *
 * This module defines the interface to the necessary system functions used by the software.
 */
#ifndef COMSYS_COMMON_SYSTEM_ADAPTER_H_
#define COMSYS_COMMON_SYSTEM_ADAPTER_H_

#include "common/comdef_common_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Fatal error function.
 *
 * This function returns the program execution to the operating system. This function is called in case of a fatal internal error.
 *
 * @param [in] error_reason Reason of the fatal error. Valid range: ::comdef_kMin <= value < ::comdef_kMax.
 */
void comsys_FatalError(const comdef_ReturnCode error_reason);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // COMSYS_COMMON_SYSTEM_ADAPTER_H_
