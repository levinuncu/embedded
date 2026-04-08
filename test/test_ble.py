import asyncio
import struct
from bleak import BleakClient, BleakScanner

## Copilot: Testet die BLE-Kommunikation mit dem ESP32 ##

# UUIDs und Namen des Zielgeräts (ESP32)
CHAR_UUID = "0000fff1-0000-1000-8000-00805f9b34fb"
TARGET_NAME = "ble_piezofly_test"
TARGET_SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb"
# Anzahl Sekunden, die auf Notifications gewartet wird
NOTIFY_SECONDS = 20

# Filtert nach dem Gerätenamen oder der Service-UUID, um das Zielgerät zu finden
def device_filter(d, ad) -> bool:
    name_ok = d.name is not None and TARGET_NAME.lower() in d.name.lower()
    service_uuids = [uuid.lower() for uuid in (ad.service_uuids or [])]
    service_ok = TARGET_SERVICE_UUID in service_uuids
    return name_ok or service_ok

# Callback-Funktion für eine ankommende Notification, empfängt das Datenpaket und entpackt die Messdaten
def on_notify(_sender, data: bytearray) -> None:
    if len(data) < 8:
        print(f"Ungueltiges Paket ({len(data)} Bytes): {data.hex()}")
        return

    temperature, humidity = struct.unpack("<ff", data[:8])
    print(f"Notify -> temp={temperature:.2f} C, hum={humidity:.2f} %")

# Asynchrone Funktion, die Bleak verwendet, um das Zielgerät zu finden, sich zu verbinden, Notifications 
# zu abonnieren und die Daten zu empfangen
async def main():
    print("Suche nach BLE-Sensor...")

    # Suche nach dem Zielgerät
    device = await BleakScanner.find_device_by_filter(device_filter, timeout=20.0)

    # Debug-Ausgabe, falls kein Gerät gefunden wurde
    if device is None:
        print("Kein passendes Geraet gefunden (Name/Service).")
        print("Gefundene BLE-Geraete (Debug):")
        for d in await BleakScanner.discover(timeout=5.0):
            print(f" - {d.name} ({d.address})")
        return

    # Debug-Ausgabe, falls das Gerät gefunden wurde
    print(f"Gefunden: {device.name} ({device.address})")

    # Verbindung zum Gerät herstellen und Notifications abonnieren
    async with BleakClient(device.address) as client:
        print("Verbunden:", client.is_connected)

        try:
            # Abonniert Notifications für die Characteristic mit der angegebenen UUID für die Dauer von 
            # NOTIFY_SECONDS Sekunden, versucht in dieser Zeit Sensordaten zu empfangen und gibt sie aus
            await client.start_notify(CHAR_UUID, on_notify)
            print(f"Warte {NOTIFY_SECONDS}s auf Notifications...")
            await asyncio.sleep(NOTIFY_SECONDS)
            await client.stop_notify(CHAR_UUID)
        except Exception as e:
            print("Fehler bei Notify:", e)

    print("Verbindung getrennt.")

# Start
asyncio.run(main())
