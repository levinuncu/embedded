import type { SensorReading } from "../types/readings";
import { HHN_LAT, HHN_LNG } from "./createFakeSensorReadings";

export function fillMissingSensorReadings(
  existingReadings: SensorReading[],
  incomingReadings: SensorReading[],
): SensorReading[] {
  const lastKnown = getLastKnownValues(existingReadings);
  const defaultLastTimestampMs = Date.now();

  return incomingReadings.map((reading, index) => {
    const filledGnss = fillGnss(
      reading,
      lastKnown,
      index,
      incomingReadings.length,
      defaultLastTimestampMs,
    );

    const filledReading: SensorReading = {
      gnss: filledGnss,
      imu: {
        accelerationX: fillNumber(
          reading.imu.accelerationX,
          lastKnown.imu.accelerationX,
          0,
          0.3,
          2,
        ),
        accelerationY: fillNumber(
          reading.imu.accelerationY,
          lastKnown.imu.accelerationY,
          0,
          0.3,
          2,
        ),
        accelerationZ: fillNumber(
          reading.imu.accelerationZ,
          lastKnown.imu.accelerationZ,
          0,
          0.3,
          2,
        ),
        gyroscopeX: fillNumber(
          reading.imu.gyroscopeX,
          lastKnown.imu.gyroscopeX,
          1,
          2,
          1,
        ),
        gyroscopeY: fillNumber(
          reading.imu.gyroscopeY,
          lastKnown.imu.gyroscopeY,
          1,
          2,
          1,
        ),
        gyroscopeZ: fillNumber(
          reading.imu.gyroscopeZ,
          lastKnown.imu.gyroscopeZ,
          1,
          2,
          1,
        ),
      },
      temperature: {
        humidity: fillNumber(
          reading.temperature.humidity,
          lastKnown.temperature.humidity,
          50,
          1,
          1,
        ),
        temperature: fillNumber(
          reading.temperature.temperature,
          lastKnown.temperature.temperature,
          27,
          1,
          1,
        ),
      },
      current: {
        milliAmpere: fillNumber(
          reading.current.milliAmpere,
          lastKnown.current.milliAmpere,
          700,
          25,
          0,
        ),
      },
    };

    updateLastKnownValues(lastKnown, filledReading);

    return filledReading;
  });
}

function getLastKnownValues(readings: SensorReading[]): SensorReading {
  const lastKnown: SensorReading = {
    temperature: {
      humidity: null,
      temperature: null,
    },
    gnss: {
      latitude: null,
      longitude: null,
      timestamp: null,
    },
    imu: {
      accelerationX: null,
      accelerationY: null,
      accelerationZ: null,
      gyroscopeX: null,
      gyroscopeY: null,
      gyroscopeZ: null,
    },
    current: {
      milliAmpere: null,
    },
  };

  for (const reading of readings) {
    updateLastKnownValues(lastKnown, reading);
  }

  return lastKnown;
}

function updateLastKnownValues(
  lastKnown: SensorReading,
  reading: SensorReading,
): void {
  if (reading.gnss.latitude !== null) {
    lastKnown.gnss.latitude = reading.gnss.latitude;
  }

  if (reading.gnss.longitude !== null) {
    lastKnown.gnss.longitude = reading.gnss.longitude;
  }

  if (reading.gnss.timestamp !== null) {
    lastKnown.gnss.timestamp = reading.gnss.timestamp;
  }

  if (reading.temperature.temperature !== null) {
    lastKnown.temperature.temperature = reading.temperature.temperature;
  }

  if (reading.temperature.humidity !== null) {
    lastKnown.temperature.humidity = reading.temperature.humidity;
  }

  if (reading.imu.accelerationX !== null) {
    lastKnown.imu.accelerationX = reading.imu.accelerationX;
  }

  if (reading.imu.accelerationY !== null) {
    lastKnown.imu.accelerationY = reading.imu.accelerationY;
  }

  if (reading.imu.accelerationZ !== null) {
    lastKnown.imu.accelerationZ = reading.imu.accelerationZ;
  }

  if (reading.imu.gyroscopeX !== null) {
    lastKnown.imu.gyroscopeX = reading.imu.gyroscopeX;
  }

  if (reading.imu.gyroscopeY !== null) {
    lastKnown.imu.gyroscopeY = reading.imu.gyroscopeY;
  }

  if (reading.imu.gyroscopeZ !== null) {
    lastKnown.imu.gyroscopeZ = reading.imu.gyroscopeZ;
  }

  if (reading.current.milliAmpere !== null) {
    lastKnown.current.milliAmpere = reading.current.milliAmpere;
  }
}

function fillNumber(
  value: null | number,
  fallback: null | number,
  defaultValue: number,
  maxDeviation: number,
  decimals: number,
): number {
  if (value !== null) {
    return value;
  }

  const baseValue = fallback ?? defaultValue;
  const drift = randomBetween(-maxDeviation, maxDeviation);

  return roundToDecimals(baseValue + drift, decimals);
}

function randomBetween(min: number, max: number): number {
  return min + Math.random() * (max - min);
}

function roundToDecimals(value: number, decimals: number): number {
  const factor = 10 ** decimals;
  return Math.round(value * factor) / factor;
}

function fillGnss(
  reading: SensorReading,
  lastKnown: SensorReading,
  index: number,
  numberOfIncomingReadings: number,
  defaultLastTimestampMs: number,
): SensorReading["gnss"] {
  const baseLatitude = lastKnown.gnss.latitude ?? HHN_LAT;
  const baseLongitude = lastKnown.gnss.longitude ?? HHN_LNG;

  const latitude =
    reading.gnss.latitude ?? addRandomMeterOffsetToLatitude(baseLatitude, 5);

  const longitude =
    reading.gnss.longitude ??
    addRandomMeterOffsetToLongitude(baseLongitude, latitude, 5);

  const timestamp =
    reading.gnss.timestamp ??
    createSyntheticTimestamp(
      index,
      numberOfIncomingReadings,
      defaultLastTimestampMs,
    );

  return {
    latitude,
    longitude,
    timestamp,
  };
}

function createSyntheticTimestamp(
  index: number,
  numberOfIncomingReadings: number,
  defaultLastTimestampMs: number,
): Date {
  const secondsBeforeLast = numberOfIncomingReadings - 1 - index;
  return new Date(defaultLastTimestampMs - secondsBeforeLast * 1000);
}

function addRandomMeterOffsetToLatitude(
  latitude: number,
  maxDeviationMeters: number,
): number {
  const meters = randomBetween(-maxDeviationMeters, maxDeviationMeters);
  const latitudeOffset = meters / 111_320;

  return roundToDecimals(latitude + latitudeOffset, 6);
}

function addRandomMeterOffsetToLongitude(
  longitude: number,
  latitude: number,
  maxDeviationMeters: number,
): number {
  const meters = randomBetween(-maxDeviationMeters, maxDeviationMeters);
  const latitudeInRadians = latitude * (Math.PI / 180);
  const metersPerLongitudeDegree = 111_320 * Math.cos(latitudeInRadians);

  if (metersPerLongitudeDegree === 0) {
    return roundToDecimals(longitude, 6);
  }

  const longitudeOffset = meters / metersPerLongitudeDegree;

  return roundToDecimals(longitude + longitudeOffset, 6);
}
