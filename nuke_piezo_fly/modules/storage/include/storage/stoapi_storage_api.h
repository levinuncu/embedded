/**
 * @file stoapi_storage_api.h
 *
 * @addtogroup storage_api
 * @{
 *
 * @brief Interface of the storage API.
 * 
 * This module defines and implements the interface functions for the storage.
 */
#ifndef STOAPI_STORAGE_API_H_
#define STOAPI_STORAGE_API_H_

#include <stddef.h>

#include "common/comdef_common_definitions.h"
#include "storage/stocty_storage_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the storage api module.
 *
 * This function is used to initialize the storage module. If the storage module is already initialized, a ::comdef_kAlreadyInitialized error is returned.
 * The passed pointer to the storage configuration is saved internally.
 * 
 * @param [in] storage_configuration Pointer to the storage configuration. If the pointer is NULL, a ::comdef_kInvalidParameter error is returned.
 * @return ::comdef_kNoError -> successful initialized
 * @return ::comdef_kAlreadyInitialized -> module already initialized
 * @return ::comdef_kInvalidParameter -> invalid parameter
 * @return ::comdef_kInternalError -> failed to initialize the flash storage
 */
comdef_ReturnCode stoapi_Init(const stocty_StorageConfiguration *const storage_configuration);

/**
 * @brief Write data to the storage.
 * 
 * This functions writes data to the storage in the configured namespace. If the storage module is not initialized, a ::comdef_kNotInitialized error is returned.
 * 
 * @param [in] data Pointer to the data. If the pointer is NULL, a ::comdef_kInvalidParameter error is returned.
 * @param [in] data_size Size of the data.
 * @return ::comdef_kNoError -> successful operation
 * @return ::radef_kNotInitialized -> module not initialized
 * @return ::comdef_kInvalidParameter -> invalid parameter
 * @return ::comdef_kInternalError -> failed to write the data
 */
comdef_ReturnCode stoapi_WriteToStorage(const void *const data, const size_t data_size);

/**
 * @brief Read data from the storage.
 * 
 * This functions reads data from the storage in the configured namespace. If the storage module is not initialized, a ::comdef_kNotInitialized error is returned.
 * If the external buffer is to small a ::comdef_kBufferTooSmall error is returned.
 * 
 * @param [in] buffer_size Size of the external buffer.
 * @param [out] buffer Pointer to the buffer. If the pointer is NULL, a ::comdef_kInvalidParameter error is returned.
 * @param [out] data_size Size of the read data. If the pointer is NULL, a ::comdef_kInvalidParameter error is returned.
 * @return ::comdef_kNoError -> successful operation
 * @return ::radef_kNotInitialized -> module not initialized
 * @return ::comdef_kInvalidParameter -> invalid parameter
 * @return ::comdef_kBufferTooSmall -> external buffer too small
 * @return ::comdef_kInternalError -> failed to read the data
 */
comdef_ReturnCode stoapi_ReadFromStorage(const size_t buffer_size, void *const buffer, size_t *const data_size);

/**
 * @brief Clear the storage.
 * 
 * This functions clears the whole storage in the configured namespace. If the storage module is not initialized, a ::comdef_kNotInitialized error is returned.
 * 
 * @return ::comdef_kNoError -> successful operation
 * @return ::radef_kNotInitialized -> module not initialized
 * @return ::comdef_kInternalError -> failed to clear the storage
 */
comdef_ReturnCode stoapi_ClearStorage(void);

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // STOAPI_STORAGE_API_H_
