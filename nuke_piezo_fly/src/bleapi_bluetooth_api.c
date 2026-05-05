/**
 * @brief Implementation of the bluetooth module.
 */
#include "bleapi_bluetooth_api.h"

#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/projdefs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "esp_err.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static void OnSync(void);
static void OnReset(int reason);
static void HostTask(void *param);
static esp_err_t GattInit(void);
static void Advertise(void);
static int CharacteristicsAccess(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int GapEvent(struct ble_gap_event *event, void *arg);

static uint8_t own_addr_type;
static uint16_t value_handle;
static uint16_t conn_handle;

/**
 * @brief Tag of the ESP logger.
 */
static const char *const kLoggerTag = "BLEAPI";

/**
 * @brief Name of the device.
 */
static const char *const kDeviceName = "piezo_fly";

/**
 * @brief GATT services.
 */
static const struct ble_gatt_svc_def kGattServices[] = {
  {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID16_DECLARE(0xFFF0),
    .characteristics = (struct ble_gatt_chr_def[]) {
      {
        .uuid = BLE_UUID16_DECLARE(0xFFF1),
        .access_cb = CharacteristicsAccess,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &value_handle,
      },
      {0},
    },
  },
  {0},
};

/**
 * @brief Initialization state of the module.
 */
static bool initialized = false;

/**
 * @brief Indicates that a client is connected.
 */
static bool client_connected = false;

/**
 * @brief Indicates if a client has subscribed.
 */
static bool notify_enabled = false;

esp_err_t bleapi_Init(void) {
  esp_err_t return_code = ESP_OK;

  return_code = nimble_port_init();
  if (return_code != ESP_OK) {
    return return_code;
  }

  ble_hs_cfg.reset_cb = OnReset;
  ble_hs_cfg.sync_cb = OnSync;

  return_code = GattInit();
  if (return_code != ESP_OK) {
    return return_code;
  }

  nimble_port_freertos_init(HostTask);

  initialized = true;

  ESP_LOGI(kLoggerTag, "Initialized");
  return return_code;
}

void bleapi_Deinit(void) {
  if (!initialized) {
    return;
  }

  if (client_connected) {
    ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
  }

  (void)ble_gap_adv_stop();
  ble_svc_gatt_deinit();
  ble_svc_gap_deinit();
  (void)nimble_port_stop();
  (void)nimble_port_deinit();

  client_connected = false;
  notify_enabled = false;
  initialized = false;

  ESP_LOGI(kLoggerTag, "Deinitialized");
}

bool bleapi_IsClientConnected(void) {
    return client_connected;
}

bool bleapi_IsNotifyEnabled(void) {
    return notify_enabled;
}

void bleapi_SendSensorsReadings(const senaty_SensorsReading *const sensors_readings, const size_t number_of_readings) {
  if (!initialized || !client_connected || !notify_enabled) {
    return;
  }

  const size_t kMaxReadingsPerPacket = 7;
  size_t offset = 0;

  while (offset < number_of_readings) {
      size_t remaining = number_of_readings - offset;
      size_t chunk_count = remaining;

      if (chunk_count > kMaxReadingsPerPacket) {
          chunk_count = kMaxReadingsPerPacket;
      }

      const size_t chunk_size_bytes = sizeof(senaty_SensorsReading) * chunk_count;
      const uint8_t *chunk_ptr = (const uint8_t *)&sensors_readings[offset];

      struct os_mbuf *notification_buffer = ble_hs_mbuf_from_flat(chunk_ptr, chunk_size_bytes);
      if (notification_buffer == NULL) {
          ESP_LOGE(kLoggerTag, "Failed to allocate mbuf");
          return;
      }

      const int kNotificationResult = ble_gatts_notify_custom(conn_handle, value_handle, notification_buffer);
      if (kNotificationResult != 0) {
          ESP_LOGE(kLoggerTag, "Notify failed, return code: %i", kNotificationResult);
      }

      offset += chunk_count;
      vTaskDelay(pdMS_TO_TICKS(100));
  }
}

static void OnReset(int reason) {
  ESP_LOGE(kLoggerTag, "Reset, reason: %i", reason);
}

static void OnSync(void) {
  int return_code = ble_hs_id_infer_auto(0, &own_addr_type);
  if (return_code != 0) {
    ESP_LOGE(kLoggerTag, "Failed to determine address type, return code: %i", return_code);
    return;
  }

  Advertise();
}

static void HostTask(void *param) {
  nimble_port_run();
  nimble_port_freertos_deinit();
}

static esp_err_t GattInit(void) {
  int return_code;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  return_code = ble_svc_gap_device_name_set(kDeviceName);
  if (return_code != 0) {
      return ESP_FAIL;
  }

  return_code = ble_gatts_count_cfg(kGattServices);
  if (return_code != 0) {
      return ESP_FAIL;
  }

  return_code = ble_gatts_add_svcs(kGattServices);
  if (return_code != 0) {
      return ESP_FAIL;
  }

  return ESP_OK;
}

static void Advertise(void) {
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *)kDeviceName;
    fields.name_len = strlen(kDeviceName);
    fields.name_is_complete = 1;

    int return_code = ble_gap_adv_set_fields(&fields);
    if (return_code != 0) {
        ESP_LOGE(kLoggerTag, "Failed to set the advertisement fields, return code: %i", return_code);
        return;
    }

    struct ble_gap_adv_params params;
    memset(&params, 0, sizeof(params));

    params.conn_mode = BLE_GAP_CONN_MODE_UND;
    params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    return_code = ble_gap_adv_start(
        own_addr_type,
        NULL,
        BLE_HS_FOREVER,
        &params,
        GapEvent,
        NULL
    );
    if (return_code != 0) {
      ESP_LOGE(kLoggerTag, "Failed to start the advertisement, return code: %i", return_code);
    } else {
      ESP_LOGI(kLoggerTag, "Advertisement started");
    }
}

static int CharacteristicsAccess(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
      const char msg[] = "BLEAPI data notify characteristic";
      int rc = os_mbuf_append(ctxt->om, msg, sizeof(msg) - 1);
      return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
      return 0;
    default:
      return BLE_ATT_ERR_UNLIKELY;
  }
}

static int GapEvent(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                client_connected = true;
                notify_enabled = false;
                conn_handle = event->connect.conn_handle;

                ESP_LOGI(kLoggerTag, "Client connected");
            } else {
                Advertise();
            }

            return 0;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(kLoggerTag, "Client disconnected");

            client_connected = false;
            notify_enabled = false;
            conn_handle = 0;

            Advertise();
            return 0;
        case BLE_GAP_EVENT_SUBSCRIBE:
            if (event->subscribe.attr_handle == value_handle) {
                notify_enabled = event->subscribe.cur_notify;
                ESP_LOGI(kLoggerTag, "Notify %s", notify_enabled ? "enabled" : "disabled");
            }

            return 0;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            Advertise();
            return 0;
        default:
            return 0;
    }
}