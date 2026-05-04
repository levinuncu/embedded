/**
 * @file stocty_storage_config_types.h
 *
 * @addtogroup storage_config_types
 * @{
 *
 * @brief Type definitions of the storage configuration.
 *
 * This module defines the data types and data structures used for the configuration of the storage.
 */
#ifndef STOCTY_STORAGE_CONFIG_TYPES_H_
#define STOCTY_STORAGE_CONFIG_TYPES_H_

#include "nvs.h"

/**
 * @brief Struct for the configuration of the storage.
 */
typedef struct {
  /**
   * @brief Name of the namespace.
   */
  char namespace_name[NVS_NS_NAME_MAX_SIZE];
} stocty_StorageConfiguration;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/** @}*/

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // STOCTY_STORAGE_CONFIG_TYPES_H_
