/**
 * @brief Type definitions for the configuration of the storage.
 */
#ifndef STOCTY_STORAGE_CONFIG_TYPES_H_
#define STOCTY_STORAGE_CONFIG_TYPES_H_

#include "nvs.h"

/**
 * @brief Configuration of the storage.
 */
typedef struct {
  /**
   * @brief Name of the namespace.
   */
  char namespace_name[NVS_NS_NAME_MAX_SIZE];
} stocty_StorageConfiguration;

#endif // STOCTY_STORAGE_CONFIG_TYPES_H_
