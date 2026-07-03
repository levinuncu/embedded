/**
 * @brief Implementation of the the storage api module.
 */
#include "storage/stoapi_storage_api.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"

#include "storage/stocty_storage_config_types.h"

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "STOAPI";

/**
 * @brief Initialization state of the module.
 */
static bool initialized = false;

/**
 * @brief Configuration of the storage.
 */
static stocty_StorageConfiguration configuration;

/**
 * @brief Storage key of the blob index.
 */
static const char kBlobIndexStorageKey[NVS_NS_NAME_MAX_SIZE] = "blob_index";

static nvs_handle_t storage_handle = UINT32_MAX;

void stoapi_Init(const stocty_StorageConfiguration storage_configuration) {
  configuration = storage_configuration;

  const esp_err_t kInitResult = nvs_flash_init();
  if (kInitResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to initialize the storage: %s", esp_err_to_name(kInitResult));
    return;
  }

  const esp_err_t kOpenResult = nvs_open(configuration.namespace_name, NVS_READWRITE, &storage_handle);
  if (kOpenResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to open the storage: %s", esp_err_to_name(kOpenResult));
    return;
  }

  ESP_LOGI(kLoggerTag, "Initialized");
  initialized = true;
}

void stoapi_Deinit(void) {
  if (storage_handle != UINT32_MAX) {
    nvs_close(storage_handle);
    storage_handle = UINT32_MAX;
  }

  ESP_LOGI(kLoggerTag, "Deinitialized");
  initialized = false;
}

void stoapi_WriteToStorage(const void *const data, const size_t data_size) {
  if (!initialized) {
    return;
  }

  // The blob index is used to create unique keys for appended records.
  uint32_t blob_index = 0;
  const esp_err_t kGetBlobIndexResult = nvs_get_u32(storage_handle, kBlobIndexStorageKey, &blob_index);
  if ((kGetBlobIndexResult != ESP_OK) && (kGetBlobIndexResult != ESP_ERR_NVS_NOT_FOUND)) {
    ESP_LOGE(kLoggerTag, "Failed to get the blob index: %s", esp_err_to_name(kGetBlobIndexResult));
    return;
  }

  char blob_key[NVS_NS_NAME_MAX_SIZE];
  // Store every record under a separate key, e.g. rd_0, rd_1, rd_2.
  (void)snprintf(blob_key, sizeof(blob_key), "rd_%lu", blob_index);

  const esp_err_t kSetBlobResult = nvs_set_blob(storage_handle, blob_key, data, data_size);
  if (kSetBlobResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to set the blob: %s", esp_err_to_name(kSetBlobResult));
    return;
  }

  ++blob_index;
  // Persist the next free blob index for the following write operation.
  const esp_err_t kSetBlobIndexResult = nvs_set_u32(storage_handle, kBlobIndexStorageKey, blob_index);     
  if (kSetBlobIndexResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to set the blob index: %s", esp_err_to_name(kSetBlobIndexResult));
    return;
  }

  const esp_err_t kCommitResult = nvs_commit(storage_handle);
  if (kCommitResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to commit the storage: %s", esp_err_to_name(kCommitResult));
    return;
  }
}

void *stoapi_ReadFromStorage(size_t *const data_size) {
  if (!initialized) {
    *data_size = 0;
    return NULL;
  }

  *data_size = 0;

  uint32_t blob_index = 0;
  // blob_index contains the next free index, so the newest stored blob is index - 1.
  const esp_err_t kGetBlobIndexResult = nvs_get_u32(storage_handle, kBlobIndexStorageKey, &blob_index);
  if (kGetBlobIndexResult == ESP_ERR_NVS_NOT_FOUND) {
    *data_size = 0;
    return NULL;
  } else if (kGetBlobIndexResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to get the blob index: %s", esp_err_to_name(kGetBlobIndexResult));
    return NULL;
  }

  char blob_key[NVS_NS_NAME_MAX_SIZE];
  void *data = NULL;

  // Read blobs from newest to oldest and append them into one continuous buffer.
  for (int32_t index = (int32_t)blob_index - 1; index >= 0; --index) {
    (void)snprintf(blob_key, sizeof(blob_key), "rd_%li", index);
    size_t blob_size = 0; 

    // First call only queries the blob size so enough memory can be allocated.
    const esp_err_t kGetBlobSizeResult = nvs_get_blob(storage_handle, blob_key, NULL, &blob_size);
    if (kGetBlobSizeResult == ESP_ERR_NVS_NOT_FOUND) {
      // Missing blob keys are skipped so reading still works after partial deletes.
      continue;
    } else if (kGetBlobSizeResult != ESP_OK) {
      ESP_LOGE(kLoggerTag, "Failed to get the blob size: %s", esp_err_to_name(kGetBlobSizeResult));
      free(data);
      *data_size = 0;
      return NULL;
    } else {
      // Nothing to do here.
    }

    data = realloc(data, *data_size + blob_size);
    // Append the current blob directly after the data already read.
    const esp_err_t kGetBlobResult = nvs_get_blob(storage_handle, blob_key, (uint8_t *)data + *data_size, &blob_size);
    if (kGetBlobResult != ESP_OK) {
      ESP_LOGE(kLoggerTag, "Failed to get the blob: %s", esp_err_to_name(kGetBlobResult));
      free(data);
      *data_size = 0;
      return NULL;
    }

    *data_size += blob_size;
  }

  return data;
}

void stoapi_ClearStorage(void) {
  if (!initialized) {
    return;
  }

  const esp_err_t kEraseResult = nvs_erase_all(storage_handle);
  if (kEraseResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to erase the storage: %s", esp_err_to_name(kEraseResult));
    return;
  }

  const esp_err_t kCommitResult = nvs_commit(storage_handle);
  if (kCommitResult != ESP_OK) {
    ESP_LOGE(kLoggerTag, "Failed to commit the storage: %s", esp_err_to_name(kCommitResult));
    return;
  }
}
