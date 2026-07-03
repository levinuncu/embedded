import type { SensorReading } from "../types/readings";

const PACKET_LEN = 32;
const MAX_PACKETS = 6;
const MAX_PACKETS_LEN = PACKET_LEN * MAX_PACKETS;

const MAX_INT8 = 0x7f;
const MAX_UINT8 = 0xff;
const MAX_INT16 = 0x7fff;
const MAX_UINT32 = 0xffffffff;
const MAX_UINT64 = 0xffffffffffffffffn;

const GNSS_SCALE = 10000;

export function parseSensorReadings(dataView: DataView): SensorReading[] {
  if (
    dataView.byteLength < PACKET_LEN ||
    dataView.byteLength % PACKET_LEN != 0 ||
    dataView.byteLength > MAX_PACKETS_LEN
  ) {
    console.log(
      `Received invalid BLE packet with ${dataView.byteLength} bytes`,
    );
    return [];
  }

  const readings: SensorReading[] = [];
  const numberOfReadings = dataView.byteLength / PACKET_LEN;

  for (let i = 0; i < numberOfReadings; i++) {
    const offset = i * PACKET_LEN;
    readings.push(parseSensorReading(dataView, offset));
  }

  return readings;
}

function parseSensorReading(dataView: DataView, offset: number): SensorReading {
  const longitudeRaw = dataView.getUint32(offset + 0, true);
  const latitudeRaw = dataView.getUint32(offset + 4, true);
  const timestampRaw = dataView.getBigUint64(offset + 8, true);

  const accelerationXRaw = dataView.getInt8(offset + 16);
  const accelerationYRaw = dataView.getInt8(offset + 17);
  const accelerationZRaw = dataView.getInt8(offset + 18);

  const gyroscopeXRaw = dataView.getInt16(offset + 20, true);
  const gyroscopeYRaw = dataView.getInt16(offset + 22, true);
  const gyroscopeZRaw = dataView.getInt16(offset + 24, true);

  const humidityRaw = dataView.getUint8(offset + 26);
  const temperatureRaw = dataView.getInt8(offset + 27);

  const currentRaw = dataView.getInt16(offset + 28, true);

  return {
    gnss: {
      longitude: decodeLongitude(longitudeRaw),
      latitude: decodeLatitude(latitudeRaw),
      timestamp: decodeTimestamp(timestampRaw),
    },
    imu: {
      accelerationX: decodeInt8(accelerationXRaw),
      accelerationY: decodeInt8(accelerationYRaw),
      accelerationZ: decodeInt8(accelerationZRaw),
      gyroscopeX: decodeInt16(gyroscopeXRaw),
      gyroscopeY: decodeInt16(gyroscopeYRaw),
      gyroscopeZ: decodeInt16(gyroscopeZRaw),
    },
    temperature: {
      humidity: decodeUint8(humidityRaw),
      temperature: decodeInt8(temperatureRaw),
    },
    current: {
      milliAmpere: decodeInt16(currentRaw),
    },
  };
}

function decodeLongitude(raw: number): number | null {
  if (raw === MAX_UINT32) {
    return null;
  }

  const isWest = raw >= 0x80000000;
  const value = isWest ? raw - 0x80000000 : raw;
  const degrees = value / GNSS_SCALE;

  return isWest ? -degrees : degrees;
}

function decodeLatitude(raw: number): number | null {
  if (raw === MAX_UINT32) {
    return null;
  }

  const isSouth = raw >= 0x80000000;
  const value = isSouth ? raw - 0x80000000 : raw;
  const degrees = value / GNSS_SCALE;

  return isSouth ? -degrees : degrees;
}

function decodeTimestamp(raw: bigint): Date | null {
  if (raw === MAX_UINT64) {
    return null;
  }

  const milliseconds = Number(raw);

  if (!Number.isSafeInteger(milliseconds)) {
    return null;
  }

  return new Date(milliseconds);
}

function decodeInt8(value: number): number | null {
  return value === MAX_INT8 ? null : value;
}

function decodeUint8(value: number): number | null {
  return value === MAX_UINT8 ? null : value;
}

function decodeInt16(value: number): number | null {
  return value === MAX_INT16 ? null : value;
}
