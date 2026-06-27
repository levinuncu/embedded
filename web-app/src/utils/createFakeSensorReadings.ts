import type { SensorReading } from "../types/readings";
import { fillMissingSensorReadings } from "./fillMissingSensorReadings";

export const HHN_LAT = 49.12212298190261;
export const HHN_LNG = 9.21056313954808;

let fakeTimestampMs = Date.now();

export function createFakeSensorReadings(
  existingReadings: SensorReading[],
  numberOfReadings: number,
): SensorReading[] {
  const emptyReadings = Array.from({ length: numberOfReadings }, () => {
    const reading = createEmptySensorReading(new Date(fakeTimestampMs));
    fakeTimestampMs += 1000;
    return reading;
  });

  return fillMissingSensorReadings(existingReadings, emptyReadings);
}

function createEmptySensorReading(timestamp: Date): SensorReading {
  return {
    gnss: {
      longitude: null,
      latitude: null,
      timestamp,
    },
    imu: {
      accelerationX: null,
      accelerationY: null,
      accelerationZ: null,
      gyroscopeX: null,
      gyroscopeY: null,
      gyroscopeZ: null,
    },
    temperature: {
      humidity: null,
      temperature: null,
    },
    current: {
      milliAmpere: null,
    },
  };
}
