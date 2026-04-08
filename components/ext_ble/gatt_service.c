#include "host/ble_hs.h"
#include "gatt_service.h"

/* Handle für die Notification der Sensor Daten */
uint16_t sensor_data_handle;

/* Callback-Funktion für den Zugriff auf die Sensor-Daten */
static int sensor_data_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle;
    (void)attr_handle;
    (void)ctxt;
    (void)arg;

    /* Der Zugriff auf die Sensor-Daten ist nicht über diesen Handle möglich */
    return BLE_ATT_ERR_READ_NOT_PERMITTED;
}

/* Definition des GATT-Service, in der Sensordaten als Characteristics bereitgestellt werden
*   - Service UUID: 0xFFF0
*   - Characteristic UUID: 0xFFF1
*   Durch die Flag wird die Characteristic als notify gekennzeichnet, wodurch abonnierte Clients diese Daten empfangen können.
*/
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xFFF0),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0xFFF1),
                .access_cb = sensor_data_access_cb,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &sensor_data_handle,
            },
            { 0 }   // Ende der Characteristic-Definitionen
        }
    },
    { 0 }   // Ende der Service-Definitionen
};

/* Initialisiert den GATT-Service durch die NimBLE-API. */
int gatt_svr_init(void)
{
    int rc;

    /* Konfig, zählt die Ressourcen, 0 bei Erfolg */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) return rc;

    /* Registriert die spezifizierten Services im GATT-Service */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    return rc;
}
