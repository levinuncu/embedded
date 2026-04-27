#include "host/ble_hs.h"
#include "gatt_service.h"

/* Handle für die Notification der Sensor Daten */
uint16_t blegat_sensor_data_handle;

/* Callback-Funktion für den Zugriff auf die Sensor-Daten */
static int SensorDataAccessCallback(uint16_t connection_handle, uint16_t attribute_handle,
                                    struct ble_gatt_access_ctxt *context, void *argument)
{
    (void)connection_handle;
    (void)attribute_handle;
    (void)context;
    (void)argument;

    /* Der Zugriff auf die Sensor-Daten ist nicht über diesen Handle möglich */
    return BLE_ATT_ERR_READ_NOT_PERMITTED;
}

/* Definition des GATT-Service, in der Sensordaten als Characteristics bereitgestellt werden
*   - Service UUID: 0xFFF0
*   - Characteristic UUID: 0xFFF1
*   Durch das Flag wird die Characteristic als notify gekennzeichnet,
*   wodurch abonnierte Clients diese Daten empfangen können.
*/
static const struct ble_gatt_svc_def kServices[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xFFF0),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0xFFF1),
                .access_cb = SensorDataAccessCallback,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &blegat_sensor_data_handle,
            },
            {0}   /* Ende der Characteristic-Definitionen */
        }
    },
    {0}   /* Ende der Service-Definitionen */
};

/* Initialisiert den GATT-Service durch die NimBLE-API. */
int blegat_Init(void)
{
    int return_code;

    return_code = ble_gatts_count_cfg(kServices);
    if (return_code != 0) {
        return return_code;
    }

    return_code = ble_gatts_add_svcs(kServices);
    return return_code;
}
