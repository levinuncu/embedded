#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "gatt_service.h"
#include "sensor_packet.h"   
#include <string.h>


static const char *tag = "BLE";

static const char *device_name = "ble_piezofly_test";

/* Handle für die Verbindung zum Client */
static uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;

/* Vorwärtsdeklaration */
static int ble_gap_event(struct ble_gap_event *event, void *arg);
static void ble_app_on_sync(void);

/* Funktion, die den BLE-Host auf das Advertising vorbereitet */
static void ble_app_on_sync(void){
    int rc;
    uint8_t addr_type;

    /* BLE-Adresse automatisch bestimmen (public/random) */
    ble_hs_id_infer_auto(0, &addr_type);

    /* Advertising-Felder konfigurieren */
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    /* Allgemeine Sichtbarkeit und kein BR (Bluetooth Classic) */
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (const uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(tag, "ble_gap_adv_set_fields failed: %d", rc);
        return;
    }

    /* Advertising-Parameter setzen
    * UND = Undirected Connectable
    * → Jeder darf versuchen zu verbinden
    * → Aber deine Security‑Settings entscheiden, wer darf
    * GEN = General Discoverable
    * → Dein Gerät erscheint in Scans
    */
    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,   // Verbindbar
        .disc_mode = BLE_GAP_DISC_MODE_GEN,   // Allgemein sichtbar
    };

    /* Startet das Advertising (unbegrenzt)*/
    rc = ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(tag, "ble_gap_adv_start failed: %d", rc);
    }
}

/* Callback beim Eintritt verschiedener BLE-Events, wie Connect, Disconnect */
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {

    /* Was passiert, wenn sich ein Client verbindet, geht sicher, dass sich nur ein Client verbinden kann */
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            conn_handle = event->connect.conn_handle;
            ESP_LOGI(tag, "Client connected");
        } else {
            ESP_LOGI(tag, "Connection failed");
        }
        return 0;

    /* Was passiert, wenn sich ein Client trennt */
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(tag, "Client disconnected");
        conn_handle = BLE_HS_CONN_HANDLE_NONE;
        /* Advertising neu starten */
        ble_app_on_sync();
        return 0;
    }

    return 0;
}

/* Funktion, die Prozesse des NimBLE-Hosts Aufgaben, wie GATT-Events, Timers, ...
*   Hier in eigenem Thread, damit der Host nicht blockiert und weiterhin BLE-Events verarbeiten 
*   kann, während die Hauptanwendung läuft.
*/
static void ble_nimble_task(void *param)
{
    /* NimBLE Host Loop */
    nimble_port_run();
    nimble_port_freertos_deinit(); 
}

/* Initialisiert den BLE-Host */
void ble_init() {
    int rc;

    /* NVS initialisieren, speichert Keys und PHY-Kalibrierungsdaten */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialisiert NimBLE (Host und Controller) und verbindet Host mit Controller */
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
        return;
    }

    /* NimBLE Host Configuration --> Funktionieren Security-Settings schon? (TODO) */
    ble_hs_cfg.reset_cb = NULL;                                      // Kein spezieller Reset-Handler nötig
    ble_hs_cfg.sync_cb = ble_app_on_sync;                            // Wird aufgerufen, wenn der Host bereit ist
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;           // Standard-Speicherverwaltung für Bonds/Keys

    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;                // Sensor ohne Display/Tasten -> Just Works + LESC
    ble_hs_cfg.sm_bonding = 1;                                       // Bonding aktivieren -> nur autorisierte App
    ble_hs_cfg.sm_sc = 1;                                            // LE Secure Connections aktivieren (ECDH)

    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;   // ESP32 verteilt ENC + ID Keys
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID; // Client muss ENC + ID liefern

    /* Initialisiert Generic Access Profile (GAP) und Generic Attribute Service (GATT)
    *    - gibt die Anzahl der Services des GATT zurück 
    */
    rc = gatt_svr_init();
    if (rc != 0) {
        ESP_LOGE(tag, "gatt_svr_init failed: %d", rc);
        return;
    }

    /* Setzt den Gerätenamen */
    rc = ble_svc_gap_device_name_set(device_name);
    if (rc != 0) {
        ESP_LOGE(tag, "ble_svc_gap_device_name_set failed: %d", rc);
        return;
    }

    /* Da die einzelnen NimBLE-Tasks laufen müssen, ohne das System zu blockieren, muss ein FreeRTOS-Task initialisiert werden */
    nimble_port_freertos_init(ble_nimble_task);
}

/* Hauptfunktion des BLE-Moduls, das die Sensordaten in die Characteristics schreibt und diese published */
void send_ble_data(struct sensor_packet_t *packet) {
    if (conn_handle == BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGW(tag, "No active connection");
        return;
    }

    struct os_mbuf *om = ble_hs_mbuf_from_flat(packet, sizeof(*packet));
    if (!om) {
        ESP_LOGE(tag, "Failed to allocate mbuf");
        return;
    }

    int rc = ble_gatts_notify_custom(conn_handle, sensor_data_handle, om);
    if (rc != 0) {
        ESP_LOGE(tag, "Notify failed: %d", rc);
    }
}