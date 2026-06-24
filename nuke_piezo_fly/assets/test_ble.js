const noble = require("@abandonware/noble");

// UUIDs und Namen des Zielgeräts
const TARGET_NAME = "piezo_fly";

// Noble arbeitet meist mit kurzen UUIDs ohne Bindestriche.
// Deine UUIDs:
// 0000fff0-0000-1000-8000-00805f9b34fb -> fff0
// 0000fff1-0000-1000-8000-00805f9b34fb -> fff1
const TARGET_SERVICE_UUID = "fff0";
const CHAR_UUID = "fff1";

const NOTIFY_SECONDS = 20;
const MAX_PACKET_BYTES = 192;
const READING_SIZE_BYTES = 32;

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function normalizeUuid(uuid) {
  if (!uuid) return "";

  const clean = uuid.toLowerCase().replaceAll("-", "");

  // 16-bit UUID aus vollem Bluetooth-Base-UUID-Format extrahieren
  if (clean.startsWith("0000") && clean.endsWith("00001000800000805f9b34fb")) {
    return clean.slice(4, 8);
  }

  return clean;
}

function deviceMatches(peripheral) {
  const name = peripheral.advertisement?.localName || peripheral.name || "";
  const serviceUuids = peripheral.advertisement?.serviceUuids || [];

  const nameOk = name.toLowerCase().includes(TARGET_NAME.toLowerCase());
  const serviceOk = serviceUuids
    .map(normalizeUuid)
    .includes(TARGET_SERVICE_UUID);

  return nameOk || serviceOk;
}

function onNotify(data) {
  // data ist ein Node.js Buffer. Buffer ist für binäre Daten gedacht.
  // Für Little-Endian-Werte nutzen wir readUInt32LE/readBigUInt64LE.
  const length = data.length;

  if (
    length < READING_SIZE_BYTES ||
    length % READING_SIZE_BYTES !== 0 ||
    length > MAX_PACKET_BYTES
  ) {
    console.log(`Ungueltiges Paket (${length} Bytes): ${data.toString("hex")}`);
    return;
  }

  const numberOfReadings = length / READING_SIZE_BYTES;

  console.log(`Received ${numberOfReadings} readings`);
  console.log("--------------");

  for (let i = 0; i < numberOfReadings; i++) {
    const offset = i * READING_SIZE_BYTES;

    // Entspricht Python: struct.unpack("<LLQ", reading[:16])
    const longitude = data.readUInt32LE(offset);
    const latitude = data.readUInt32LE(offset + 4);
    const timestamp = data.readBigUInt64LE(offset + 8);

    console.log(`Reading ${i + 1}`);
    console.log(
      `Notify -> longitude=${longitude}, latitude=${latitude}, timestamp=${timestamp.toString()}`,
    );
    console.log("--------------");
  }
}

async function waitForBluetoothPoweredOn() {
  if (noble.state === "poweredOn") {
    return;
  }

  await new Promise((resolve, reject) => {
    const timeout = setTimeout(() => {
      noble.removeListener("stateChange", onStateChange);
      reject(new Error(`Bluetooth nicht bereit. State: ${noble.state}`));
    }, 10000);

    function onStateChange(state) {
      console.log("Bluetooth state:", state);

      if (state === "poweredOn") {
        clearTimeout(timeout);
        noble.removeListener("stateChange", onStateChange);
        resolve();
      }
    }

    noble.on("stateChange", onStateChange);
  });
}

async function findDevice(timeoutMs) {
  const discovered = [];

  console.log("Suche nach BLE-Sensor...");

  await noble.startScanningAsync([], false);

  return await new Promise((resolve) => {
    const timer = setTimeout(async () => {
      noble.removeListener("discover", onDiscover);
      await noble.stopScanningAsync().catch(() => {});

      console.log("Kein passendes Geraet gefunden (Name/Service).");
      console.log("Gefundene BLE-Geraete (Debug):");

      for (const p of discovered) {
        const name = p.advertisement?.localName || "(kein Name)";
        console.log(` - ${name} (${p.address}) RSSI=${p.rssi}`);
      }

      resolve(null);
    }, timeoutMs);

    async function onDiscover(peripheral) {
      discovered.push(peripheral);

      const name = peripheral.advertisement?.localName || "(kein Name)";
      const services = peripheral.advertisement?.serviceUuids || [];

      console.log(
        `Gefunden: ${name} (${peripheral.address}) RSSI=${peripheral.rssi} services=${services.join(",")}`,
      );

      if (!deviceMatches(peripheral)) {
        return;
      }

      clearTimeout(timer);
      noble.removeListener("discover", onDiscover);
      await noble.stopScanningAsync().catch(() => {});

      resolve(peripheral);
    }

    noble.on("discover", onDiscover);
  });
}

async function main() {
  let peripheral = null;

  try {
    await waitForBluetoothPoweredOn();

    peripheral = await findDevice(20000);

    if (!peripheral) {
      return;
    }

    const name =
      peripheral.advertisement?.localName || peripheral.name || "(kein Name)";
    console.log(`Passendes Geraet gefunden: ${name} (${peripheral.address})`);

    await peripheral.connectAsync();
    console.log("Verbunden:", peripheral.state === "connected");

    peripheral.once("disconnect", () => {
      console.log("Verbindung getrennt.");
    });

    const { characteristics } =
      await peripheral.discoverSomeServicesAndCharacteristicsAsync(
        [TARGET_SERVICE_UUID],
        [CHAR_UUID],
      );

    if (!characteristics || characteristics.length === 0) {
      throw new Error("Characteristic nicht gefunden.");
    }

    const characteristic = characteristics[0];

    console.log("Characteristic gefunden:", characteristic.uuid);
    console.log("Properties:", characteristic.properties.join(", "));

    // Notifications empfangen
    characteristic.on("data", onNotify);

    await characteristic.subscribeAsync();

    console.log(`Warte ${NOTIFY_SECONDS}s auf Notifications...`);
    await sleep(NOTIFY_SECONDS * 1000);

    await characteristic.unsubscribeAsync();
  } catch (err) {
    console.error("Fehler:", err);
  } finally {
    if (peripheral && peripheral.state === "connected") {
      await peripheral.disconnectAsync().catch(() => {});
    }

    await noble.stopScanningAsync().catch(() => {});
  }
}

main();
