/**
 * @file stoapi_storage_api.c
 *
 * @brief Implementation of the storage API.
 */

#include "storage/stoapi_storage_api.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "common/comass_common_assert.h"
#include "common/comdef_common_definitions.h"
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "storage/stocty_storage_config_types.h"

/**
 * @brief Initialization state of the module.
 */
static bool stoapi_initialized = false;

/**
 * @brief Pointer to the storage configuration.
 */
static const stocty_StorageConfiguration *stoapi_storage_configuration = NULL;

/**
 * @brief Storage key of the blob index.
 */
static const char kBlobIndexStorageKey[NVS_NS_NAME_MAX_SIZE] = "blob_index";

comdef_ReturnCode stoapi_Init(const stocty_StorageConfiguration *const storage_configuration) {
  comdef_ReturnCode return_code = comdef_kNoError;

  if (storage_configuration == NULL) {
    return_code = comdef_kInvalidParameter;
  } else if (stoapi_initialized) {
    return_code = comdef_kAlreadyInitialized;
  } else {
    const esp_err_t kFlashInitResult = nvs_flash_init();
    if (kFlashInitResult == ESP_OK) {
      stoapi_storage_configuration = storage_configuration;
      stoapi_initialized = true;
    } else {
      return_code = comdef_kInternalError;
    }
  }

  return return_code;
}

comdef_ReturnCode stoapi_WriteToStorage(const void *const data, const size_t data_size) {
  comdef_ReturnCode return_code = comdef_kNoError;

  if (!stoapi_initialized) {
    return_code = comdef_kNotInitialized;
  } else if (data == NULL) {
    return_code = comdef_kInvalidParameter;
  } else {
    comass_AssertNotNull(stoapi_storage_configuration, comdef_kInternalError);

    nvs_handle_t nvs_handle;
    const esp_err_t kOpenResult = nvs_open(stoapi_storage_configuration->namespace_name, NVS_READWRITE, &nvs_handle);
    if (kOpenResult == ESP_OK) {
      uint32_t blob_index = 0;
      const esp_err_t kGetBlobIndexResult = nvs_get_u32(nvs_handle, kBlobIndexStorageKey, &blob_index);
      if ((kGetBlobIndexResult == ESP_OK) || (kGetBlobIndexResult == ESP_ERR_NVS_NOT_FOUND)) {
        char blob_key[NVS_NS_NAME_MAX_SIZE];
        (void)snprintf(blob_key, sizeof(blob_key), "rd_%lu", blob_index);

        const esp_err_t kSetBlobResult = nvs_set_blob(nvs_handle, blob_key, data, data_size);
        if (kSetBlobResult == ESP_OK) {
          ++blob_index;
          const esp_err_t kSetBlockIndexResult = nvs_set_u32(nvs_handle, kBlobIndexStorageKey, blob_index);
          if (kSetBlockIndexResult == ESP_OK) {
            const esp_err_t kCommitResult = nvs_commit(nvs_handle);
            if (kCommitResult != ESP_OK) {
              return_code = comdef_kInternalError;
            }
          } else {
            return_code = comdef_kInternalError;
          }
        } else {
          return_code = comdef_kInternalError;
        }
      } else {
        return_code = comdef_kInternalError;
      }

      nvs_close(nvs_handle);
    } else {
      return_code = comdef_kInternalError;
    }
  }

  return return_code;
}

// TODO: Refactor this function to use heap allocated array directly inside the function body?
comdef_ReturnCode stoapi_ReadFromStorage(const size_t buffer_size, void *const buffer, size_t *const data_size) {
  comdef_ReturnCode return_code = comdef_kNoError;

  if (!stoapi_initialized) {
    return_code = comdef_kNotInitialized;
  } else if ((buffer == NULL) || (data_size == NULL)) {
    return_code = comdef_kInvalidParameter;
  } else {
    comass_AssertNotNull(stoapi_storage_configuration, comdef_kInternalError);

    nvs_handle_t nvs_handle;
    const esp_err_t kOpenResult = nvs_open(stoapi_storage_configuration->namespace_name, NVS_READONLY, &nvs_handle);
    if (kOpenResult == ESP_OK) {
      uint32_t blob_index = 0;
      const esp_err_t kGetBlobIndexResult = nvs_get_u32(nvs_handle, kBlobIndexStorageKey, &blob_index);
      if (kGetBlobIndexResult == ESP_OK) {
        char blob_key[NVS_NS_NAME_MAX_SIZE];
        size_t required_size = 0;

        *data_size = 0;
        for (int32_t index = (int32_t)blob_index - 1; index >= 0; --index) {
          (void)snprintf(blob_key, sizeof(blob_key), "rd_%li", index);

          const esp_err_t kGetBlobSizeResult = nvs_get_blob(nvs_handle, blob_key, NULL, &required_size);
          if (kGetBlobSizeResult == ESP_ERR_NVS_NOT_FOUND) {
            continue;
          } else if (kGetBlobSizeResult != ESP_OK) {
            return_code = comdef_kInternalError;
            break;
          } else {
            // Nothing to do here.
          }

          if ((*data_size + required_size) > buffer_size) {
            return_code = comdef_kBufferTooSmall;
            break;
          }

          const esp_err_t kGetBlobResult =
              nvs_get_blob(nvs_handle, blob_key, (uint8_t *)buffer + *data_size, &required_size);
          if (kGetBlobResult != ESP_OK) {
            return_code = comdef_kInternalError;
            break;
          }

          *data_size += required_size;
        }
      } else if (kGetBlobIndexResult == ESP_ERR_NVS_NOT_FOUND) {
        *data_size = 0;
      } else {
        return_code = comdef_kInternalError;
      }

      nvs_close(nvs_handle);
    } else if (kOpenResult == ESP_ERR_NVS_NOT_FOUND) {
      *data_size = 0;
    } else {
      return_code = comdef_kInternalError;
    }
  }

  return return_code;
}

comdef_ReturnCode stoapi_ClearStorage(void) {
  comdef_ReturnCode return_code = comdef_kNoError;

  if (!stoapi_initialized) {
    return_code = comdef_kNotInitialized;
  } else {
    comass_AssertNotNull(stoapi_storage_configuration, comdef_kInternalError);

    nvs_handle_t nvs_handle;
    const esp_err_t kOpenResult = nvs_open(stoapi_storage_configuration->namespace_name, NVS_READWRITE, &nvs_handle);
    if (kOpenResult == ESP_OK) {
      const esp_err_t kEraseResult = nvs_erase_all(nvs_handle);
      if (kEraseResult == ESP_OK) {
        const esp_err_t kCommitResult = nvs_commit(nvs_handle);
        if (kCommitResult != ESP_OK) {
          return_code = comdef_kInternalError;
        }
      } else {
        return_code = comdef_kInternalError;
      }

      nvs_close(nvs_handle);
    } else {
      return_code = comdef_kInternalError;
    }
  }

  return return_code;
}
