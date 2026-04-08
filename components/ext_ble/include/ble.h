#pragma once

/* Modul für BLE-Kommunikation */

#include "host/ble_hs.h"
#include "gatt_service.h"
#include "sensor_packet.h"

void ble_init();
void send_ble_data(struct sensor_packet_t *packet);