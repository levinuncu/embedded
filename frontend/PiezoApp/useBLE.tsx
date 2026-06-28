import { useCallback, useState } from 'react';
import { PermissionsAndroid, Platform } from 'react-native';
import {
    BleError,
    BleManager,
    Characteristic,
    Device,
} from 'react-native-ble-plx';
import { PERMISSIONS, requestMultiple } from 'react-native-permissions';
import DeviceInfo from 'react-native-device-info';
import AsyncStorage from '@react-native-async-storage/async-storage';

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

interface SensorData {
    avgSpeed: number | null;
    distance: number | null;
    startTime: number | null;
    elapsedTime: number;
    current: number | null;
    temperature: number | null;
    humidity: number | null;
    location: { latitude: number | null; longitude: number | null } | null;
    imu: {
        acc_x: number;
        acc_y: number;
        acc_z: number;
        gyro_x: number;
        gyro_y: number;
        gyro_z: number;
    } | null;
    timestamp: number | null;
    lastUpdatedAt: Date | null;
    isRunning: boolean;
}

type VoidCallback = (result: boolean) => void;

interface BluetoothLowEnergyApi {
    requestPermissions(cb: VoidCallback): Promise<void>;
    scanForPeripherals(): void;
    connectToDevice: (deviceId: Device) => Promise<void>;
    reconnectToDevice: (deviceId: string) => Promise<void>;
    disconnectFromDevice: () => void;
    connectedDevice: Device | null;
    allDevices: Device[];
    sensorData: SensorData;
    setSensorData: React.Dispatch<React.SetStateAction<SensorData>>;
    readings: SensorReading[];
    startStopRun: (deviceId: Device) => Promise<void>;
    formatElapsedTime: (ms: number) => string;
    loadLastSensorData: () => Promise<SensorData | null>;
    loadLastConnectedDevice: () => Promise<{ id: string; name: string } | null>;
    saveSensorData: (data: SensorData) => Promise<void>;
}

const SENSOR_UUID = '0000fff0-0000-1000-8000-00805f9b34fb';
const SENSOR_CHARACTERISTIC = '0000fff1-0000-1000-8000-00805f9b34fb';

const PACKET_LEN = 32;
const MAX_PACKETS = 6;
const MAX_PACKETS_LEN = PACKET_LEN * MAX_PACKETS;

const MAX_INT8 = 0x7f;
const MAX_UINT8 = 0xff;
const MAX_INT16 = 0x7fff;
const MAX_UINT32 = 0xffffffff;
const MAX_UINT64 = 0xffffffffffffffffn;

const GNSS_SCALE = 10000;

const HHN_LAT = 49.12287316886428;
const HHN_LNG = 9.211840988809245;

const bleManager = new BleManager();

function base64ToDataView(base64: string): DataView {
    const binaryString = atob(base64);
    const bytes = new Uint8Array(binaryString.length);
    for (let i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return new DataView(bytes.buffer);
}

function parseSensorReadings(dataView: DataView): SensorReading[] {
    if (
        dataView.byteLength < PACKET_LEN ||
        dataView.byteLength % PACKET_LEN !== 0 ||
        dataView.byteLength > MAX_PACKETS_LEN
    ) {
        console.warn(`Invalid BLE packet length: ${dataView.byteLength}`);
        return [];
    }

    const readings: SensorReading[] = [];
    const count = dataView.byteLength / PACKET_LEN;

    for (let i = 0; i < count; i++) {
        readings.push(parseSingleReading(dataView, i * PACKET_LEN));
    }

    return readings;
}

function parseSingleReading(dataView: DataView, offset: number): SensorReading {
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
    if (raw === MAX_UINT32) return null;
    const isWest = raw >= 0x80000000;
    const value = isWest ? raw - 0x80000000 : raw;
    return isWest ? -(value / GNSS_SCALE) : value / GNSS_SCALE;
}

function decodeLatitude(raw: number): number | null {
    if (raw === MAX_UINT32) return null;
    const isSouth = raw >= 0x80000000;
    const value = isSouth ? raw - 0x80000000 : raw;
    return isSouth ? -(value / GNSS_SCALE) : value / GNSS_SCALE;
}

function decodeTimestamp(raw: bigint): Date | null {
    if (raw === MAX_UINT64) return null;
    const ms = Number(raw);
    return Number.isSafeInteger(ms) ? new Date(ms) : null;
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

function randomBetween(min: number, max: number): number {
    return min + Math.random() * (max - min);
}

function mockLatitude(base: number, maxMeters = 5): number {
    const offset = randomBetween(-maxMeters, maxMeters) / 111_320;
    return Math.round((base + offset) * 1_000_000) / 1_000_000;
}

function mockLongitude(base: number, latitude: number, maxMeters = 5): number {
    const metersPerDeg = 111_320 * Math.cos(latitude * (Math.PI / 180));
    if (metersPerDeg === 0) return base;
    const offset = randomBetween(-maxMeters, maxMeters) / metersPerDeg;
    return Math.round((base + offset) * 1_000_000) / 1_000_000;
}

function fillLocation(
    reading: SensorReading,
    lastLat: number,
    lastLng: number,
): SensorReading {
    if (reading.gnss.latitude !== null && reading.gnss.longitude !== null) {
        return reading; // skips filling when real GPS data is available
    }

    const filledLat = mockLatitude(lastLat);
    const filledLng = mockLongitude(lastLng, filledLat);

    return {
        ...reading,
        gnss: {
            ...reading.gnss,
            latitude: filledLat,
            longitude: filledLng,
        },
    };
}

function readingToSensorDataFields(reading: SensorReading) {
    return {
        avgSpeed: null as number | null,
        current: reading.current.milliAmpere !== null
            ? reading.current.milliAmpere / 1000   // mA --> A
            : null,
        temperature: reading.temperature.temperature,
        humidity: reading.temperature.humidity,
        location: {
            latitude: reading.gnss.latitude,
            longitude: reading.gnss.longitude,
        },
        imu: (
            reading.imu.accelerationX !== null &&
            reading.imu.accelerationY !== null &&
            reading.imu.accelerationZ !== null &&
            reading.imu.gyroscopeX !== null &&
            reading.imu.gyroscopeY !== null &&
            reading.imu.gyroscopeZ !== null
        ) ? {
            acc_x: reading.imu.accelerationX,
            acc_y: reading.imu.accelerationY,
            acc_z: reading.imu.accelerationZ,
            gyro_x: reading.imu.gyroscopeX,
            gyro_y: reading.imu.gyroscopeY,
            gyro_z: reading.imu.gyroscopeZ,
        } : null,
        timestamp: reading.gnss.timestamp?.getTime() ?? null,
        lastUpdatedAt: new Date(),
    };
}

function haversineDistance(lat1: number, lon1: number, lat2: number, lon2: number): number {
    const toRad = (x: number) => (x * Math.PI) / 180;
    const R = 6371;
    const dLat = toRad(lat2 - lat1);
    const dLon = toRad(lon2 - lon1);
    const a =
        Math.sin(dLat / 2) ** 2 +
        Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLon / 2) ** 2;
    return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
}


function useBLE(): BluetoothLowEnergyApi {
    const [allDevices, setAllDevices] = useState<Device[]>([]);
    const [connectedDevice, setConnectedDevice] = useState<Device | null>(null);
    const [readings, setReadings] = useState<SensorReading[]>([]);
    const [sensorData, setSensorData] = useState<SensorData>({
        avgSpeed: null,
        distance: null,
        startTime: null,
        elapsedTime: 0,
        current: null,
        temperature: null,
        humidity: null,
        location: null,
        imu: null,
        timestamp: null,
        lastUpdatedAt: null,
        isRunning: false,
    });

    const requestPermissions = useCallback(async (cb: VoidCallback) => {
        if (Platform.OS === 'android') {
            const apiLevel = await DeviceInfo.getApiLevel();

            if (apiLevel < 31) {
                const granted = await PermissionsAndroid.request(
                    PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
                    {
                        title: 'Location Permission',
                        message: 'Bluetooth Low Energy requires Location',
                        buttonNeutral: 'Ask Later',
                        buttonNegative: 'Cancel',
                        buttonPositive: 'OK',
                    },
                );
                cb(granted === PermissionsAndroid.RESULTS.GRANTED);
            } else {
                const result = await requestMultiple([
                    PERMISSIONS.ANDROID.BLUETOOTH_SCAN,
                    PERMISSIONS.ANDROID.BLUETOOTH_CONNECT,
                    PERMISSIONS.ANDROID.ACCESS_FINE_LOCATION,
                ]);

                const isGranted =
                    result['android.permission.BLUETOOTH_CONNECT'] === PermissionsAndroid.RESULTS.GRANTED &&
                    result['android.permission.BLUETOOTH_SCAN'] === PermissionsAndroid.RESULTS.GRANTED &&
                    result['android.permission.ACCESS_FINE_LOCATION'] === PermissionsAndroid.RESULTS.GRANTED;

                cb(isGranted);
            }
        } else {
            cb(true);
        }
    }, []);

    const isDuplicateDevice = useCallback((devices: Device[], nextDevice: Device) =>
        devices.findIndex(device => nextDevice.id === device.id) > -1, []);

    const scanForPeripherals = useCallback(() => {
        bleManager.startDeviceScan(null, null, (error, device) => {
            if (error) {
                console.log('scan error:', error);
            }
            if (device && device.name?.includes('piezo')) {
                setAllDevices(prevState => {
                    if (!isDuplicateDevice(prevState, device)) {
                        return [...prevState, device];
                    }
                    return prevState;
                });
            }
        });
    }, [isDuplicateDevice]);

    const connectToDevice = async (device: Device) => {
        try {
            const deviceConnection = await bleManager.connectToDevice(device.id, { requestMTU: 224 });
            setConnectedDevice(deviceConnection);
            await deviceConnection.discoverAllServicesAndCharacteristics();
            bleManager.stopDeviceScan();
            getData(deviceConnection);
            saveDeviceInfo(device);
        } catch (e) {
            console.log('FAILED TO CONNECT', e);
        }
    };

    const reconnectToDevice = async (deviceId: string) => {
        try {
            const device = allDevices.find(d => d.id === deviceId);
            if (device) {
                await connectToDevice(device);
            } else {
                await scanForPeripherals();
                const foundDevice = allDevices.find(d => d.id === deviceId);
                if (foundDevice) {
                    await connectToDevice(foundDevice);
                    bleManager.stopDeviceScan();
                }
            }
        } catch (e) {
            console.error('Failed to reconnect to device', e);
        }
    };

    const onConnectedSensorsUpdate = useCallback((
        error: BleError | null,
        characteristic: Characteristic | null,
    ) => {
        if (error) {
            console.log(error);
            return;
        }
        if (!characteristic?.value) {
            console.log('no data received');
            return;
        }

        const dataView = base64ToDataView(characteristic.value);

        const bytes = new Uint8Array(dataView.buffer);
        if (bytes.every(b => b === 0)) return; // skips empty data bytes

        const newReadings = parseSensorReadings(dataView);
        if (newReadings.length === 0) return;

        // fills missing location with mock data
        setReadings(prevReadings => {
            const lastReading = prevReadings.at(-1);
            const lastLat = lastReading?.gnss.latitude ?? HHN_LAT;
            const lastLng = lastReading?.gnss.longitude ?? HHN_LNG;

            const filled = newReadings.map((r, i) => {
                const baseLat = i === 0 ? lastLat : (newReadings[i - 1].gnss.latitude ?? lastLat);
                const baseLng = i === 0 ? lastLng : (newReadings[i - 1].gnss.longitude ?? lastLng);
                return fillLocation(r, baseLat, baseLng);
            });

            return [...prevReadings, ...filled].slice(-1000); // caps data history at 1000
        });

        const rawLatest = newReadings[0];

        const isFaultyReading =
            rawLatest.temperature.temperature === null ||
            rawLatest.temperature.humidity === null;

        if (isFaultyReading) return;

        setSensorData(prev => {
            const baseLat = prev.location?.latitude ?? HHN_LAT;
            const baseLng = prev.location?.longitude ?? HHN_LNG;
            const latest = fillLocation(rawLatest, baseLat, baseLng);

            const fields = readingToSensorDataFields(latest);

            if (prev.isRunning && prev.location?.latitude != null && prev.location?.longitude != null
                && fields.location?.latitude != null && fields.location?.longitude != null) {

                const increment = haversineDistance(
                    prev.location.latitude!,
                    prev.location.longitude!,
                    fields.location.latitude,
                    fields.location.longitude,
                );

                return {
                    ...fields,
                    avgSpeed: prev.avgSpeed,
                    distance: (prev.distance ?? 0) + increment,
                    isRunning: true,
                    startTime: prev.startTime,
                    elapsedTime: Date.now() - (prev.startTime ?? Date.now()),
                };
            }

            return {
                ...fields,
                avgSpeed: prev.avgSpeed,
                distance: prev.distance,
                isRunning: prev.isRunning,
                startTime: prev.startTime,
                elapsedTime: prev.elapsedTime,
            };
        });
    }, []);

    const getData = useCallback(async (device: Device) => {
        if (device) {
            device.monitorCharacteristicForService(
                SENSOR_UUID,
                SENSOR_CHARACTERISTIC,
                (error, characteristic) => onConnectedSensorsUpdate(error, characteristic),
            );
        } else {
            console.log('no device connected');
        }
    }, [onConnectedSensorsUpdate]);

    const disconnectFromDevice = useCallback(() => {
        if (connectedDevice) {
            bleManager.cancelDeviceConnection(connectedDevice.id);
            setConnectedDevice(null);
            console.log('disconnected: ', connectedDevice.name);
        }
    }, [connectedDevice]);

    const startStopRun = async (device: Device) => {
        setSensorData(prev => {
            if (!prev.isRunning) {
                return {
                    ...prev,
                    isRunning: true,
                    startTime: Date.now(),
                    elapsedTime: 0,
                    distance: 0,
                    avgSpeed: null,
                };
            } else {
                const elapsedTime = Date.now() - (prev.startTime ?? Date.now());
                let avgSpeedValue: number | null = null;

                if (prev.distance && prev.distance > 0) {
                    avgSpeedValue = elapsedTime / prev.distance; // ms per km
                }

                return {
                    ...prev,
                    isRunning: false,
                    startTime: null,
                    elapsedTime,
                    avgSpeed: avgSpeedValue,
                };
            }
        });
    };

    function formatElapsedTime(ms: number): string {
        const totalSeconds = Math.floor(ms / 1000);
        const minutes = Math.floor(totalSeconds / 60);
        const seconds = totalSeconds % 60;
        return `${minutes}:${seconds.toString().padStart(2, '0')}`;
    }

    const saveSensorData = async (data: SensorData) => {
        if (data.lastUpdatedAt === null && data.temperature === null) return;
        try {
            await AsyncStorage.setItem('@lastSensorData', JSON.stringify(data));
        } catch (e) {
            console.error('Failed to save sensor data', e);
        }
    };

    const saveDeviceInfo = async (device: Device) => {
        try {
            await AsyncStorage.setItem('@lastConnectedDevice', JSON.stringify({
                id: device.id,
                name: device.name,
            }));
        } catch (e) {
            console.error('Failed to save device info', e);
        }
    };

    const loadLastSensorData = async () => {
        try {
            const jsonValue = await AsyncStorage.getItem('@lastSensorData');
            if (jsonValue != null) {
                const data = JSON.parse(jsonValue);
                if (data.lastUpdatedAt) {
                    data.lastUpdatedAt = new Date(data.lastUpdatedAt);
                }
                return data;
            }
            return null;
        } catch (e) {
            console.error('Failed to load sensor data', e);
            return null;
        }
    };

    const loadLastConnectedDevice = async () => {
        try {
            const jsonValue = await AsyncStorage.getItem('@lastConnectedDevice');
            return jsonValue != null ? JSON.parse(jsonValue) : null;
        } catch (e) {
            console.error('Failed to load device info', e);
            return null;
        }
    };

    return {
        scanForPeripherals,
        requestPermissions,
        connectToDevice,
        reconnectToDevice,
        allDevices,
        connectedDevice,
        disconnectFromDevice,
        sensorData,
        setSensorData,
        readings,
        startStopRun,
        formatElapsedTime,
        loadLastSensorData,
        loadLastConnectedDevice,
        saveSensorData,
    };
}

export default useBLE;