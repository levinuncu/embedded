export type SensorReading = {
  gnss: {
    longitude: null | number;
    latitude: null | number;
    timestamp: null | Date;
  };
  imu: {
    accelerationX: null | number;
    accelerationY: null | number;
    accelerationZ: null | number;
    gyroscopeX: null | number;
    gyroscopeY: null | number;
    gyroscopeZ: null | number;
  };
  temperature: {
    humidity: null | number;
    temperature: null | number;
  };
  current: {
    milliAmpere: null | number;
  };
};
