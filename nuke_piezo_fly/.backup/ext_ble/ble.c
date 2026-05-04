#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "gatt_service.h"
#include "sensor/senaty_sensor_api_types.h"
#include <string.h>

static const char *const kLoggerTag = "BLE";
static const char *const kDeviceName = "ble_piezofly_test";

/* Handle für die Verbindung zum Client */
static uint16_t bleapi_connection_handle = BLE_HS_CONN_HANDLE_NONE;

/* Vorwärtsdeklaration */
static int GapEventCallback(struct ble_gap_event *event, void *argument);
static void OnHostSynchronized(void);

/* Funktion, die den BLE-Host auf das Advertising vorbereitet */
static void OnHostSynchronized(void) {
    int return_code;
    uint8_t address_type = 0;

    /* BLE-Adresse automatisch bestimmen (public/random) */
    (void)ble_hs_id_infer_auto(0, &address_type);

    /* Advertising-Felder konfigurieren */
    struct ble_hs_adv_fields advertising_fields;
    memset(&advertising_fields, 0, sizeof(advertising_fields));
    /* Allgemeine Sichtbarkeit und kein BR (Bluetooth Classic) */
    advertising_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    advertising_fields.name = (const uint8_t *)kDeviceName;
    advertising_fields.name_len = strlen(kDeviceName);
    advertising_fields.name_is_complete = 1;

    return_code = ble_gap_adv_set_fields(&advertising_fields);
    if (return_code != 0) {
        ESP_LOGE(kLoggerTag, "ble_gap_adv_set_fields failed: %d", return_code);
        return;
    }

    /* Advertising-Parameter setzen
    * UND = Undirected Connectable
    * -> Jeder darf versuchen zu verbinden
    * -> Aber deine Security-Settings entscheiden, wer darf
    * GEN = General Discoverable
    * -> Dein Gerät erscheint in Scans
    */
    const struct ble_gap_adv_params kAdvertisingParameters = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };

    /* Startet das Advertising (unbegrenzt) */
    return_code = ble_gap_adv_start(address_type, NULL, BLE_HS_FOREVER, &kAdvertisingParameters, GapEventCallback,
                                    NULL);
    if (return_code != 0) {
        ESP_LOGE(kLoggerTag, "ble_gap_adv_start failed: %d", return_code);
    }
}

/* Callback beim Eintritt verschiedener BLE-Events, wie Connect, Disconnect */
static int GapEventCallback(struct ble_gap_event *event, void *argument) {
    (void)argument;

    switch (event->type) {
    /* Was passiert, wenn sich ein Client verbindet,
     * geht sicher, dass sich nur ein Client verbinden kann
     */
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            bleapi_connection_handle = event->connect.conn_handle;
            ESP_LOGI(kLoggerTag, "Client connected");
        } else {
            ESP_LOGI(kLoggerTag, "Connection failed");
        }
        return 0;

    /* Was passiert, wenn sich ein Client trennt */
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(kLoggerTag, "Client disconnected");
        bleapi_connection_handle = BLE_HS_CONN_HANDLE_NONE;
        /* Advertising neu starten */
        OnHostSynchronized();
        return 0;

    default:
        return 0;
    }
}

/* Funktion, die Prozesse des NimBLE-Hosts Aufgaben, wie GATT-Events, Timers, ...
*   Hier in eigenem Thread, damit der Host nicht blockiert und weiterhin BLE-Events verarbeiten
*   kann, während die Hauptanwendung läuft.
*/
static void bleapi_NimbleTask(void *parameter) {
    (void)parameter;
    /* NimBLE Host Loop */
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/* Initialisiert den BLE-Host */
void bleapi_Init(void) {
    int return_code;

    /* NVS initialisieren, speichert Keys und PHY-Kalibrierungsdaten */
    esp_err_t initialization_result = nvs_flash_init();
    if ((initialization_result == ESP_ERR_NVS_NO_FREE_PAGES) ||
        (initialization_result == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        initialization_result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(initialization_result);

    /* Initialisiert NimBLE (Host und Controller) und verbindet Host mit Controller */
    initialization_result = nimble_port_init();
    if (initialization_result != ESP_OK) {
        MODLOG_DFLT(ERROR, "Failed to init nimble %d\n", initialization_result);
        return;
    }

    /* NimBLE Host Configuration --> Funktionieren Security-Settings schon? (TODO) */
    ble_hs_cfg.reset_cb = NULL;
    ble_hs_cfg.sync_cb = OnHostSynchronized;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    /* Initialisiert Generic Access Profile (GAP) und Generic Attribute Service (GATT)
    *    - gibt die Anzahl der Services des GATT zurück
    */
    return_code = blegat_Init();
    if (return_code != 0) {
        ESP_LOGE(kLoggerTag, "blegat_Init failed: %d", return_code);
        return;
    }

    /* Setzt den Gerätenamen */
    return_code = ble_svc_gap_device_name_set(kDeviceName);
    if (return_code != 0) {
        ESP_LOGE(kLoggerTag, "ble_svc_gap_device_name_set failed: %d", return_code);
        return;
    }

    /* Da die einzelnen NimBLE-Tasks laufen müssen, ohne das System zu blockieren,
     * muss ein FreeRTOS-Task initialisiert werden
     */
    nimble_port_freertos_init(bleapi_NimbleTask);
}

/* Hauptfunktion des BLE-Moduls, das die Sensordaten in die Characteristics schreibt und published */
void bleapi_SendSensorsReading(const senaty_SensorsReading *const sensors_reading) {
    if (bleapi_connection_handle == BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGW(kLoggerTag, "No active connection");
        return;
    }

    struct os_mbuf *notification_buffer = ble_hs_mbuf_from_flat(sensors_reading, sizeof(*sensors_reading));
    if (notification_buffer == NULL) {
        ESP_LOGE(kLoggerTag, "Failed to allocate mbuf");
        return;
    }

    const int kNotificationResult =
        ble_gatts_notify_custom(bleapi_connection_handle, blegat_sensor_data_handle, notification_buffer);
    if (kNotificationResult != 0) {
        ESP_LOGE(kLoggerTag, "Notify failed: %d", kNotificationResult);
    }
}