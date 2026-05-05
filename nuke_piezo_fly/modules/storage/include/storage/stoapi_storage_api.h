/**
 * @brief Interface of the the storage api module.
 */
#ifndef STOAPI_STORAGE_API_H_
#define STOAPI_STORAGE_API_H_

#include "stocty_storage_config_types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initialize the storage api module.
 */
void stoapi_Init(const stocty_StorageConfiguration storage_configuration);

/**
 * @brief Deinitialize the storage api module.
 */
void stoapi_Deinit(void);

/**
 * @brief Write data to the storage.
 */
void stoapi_WriteToStorage(const void *const data, const size_t data_size);

/**
 * @brief Read data to the storage.
 */
void *stoapi_ReadFromStorage(size_t *const data_size);

/**
 * @brief Clear the storage.
 */
void stoapi_ClearStorage(void);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // STOAPI_STORAGE_API_H_
