/* eslint-disable no-bitwise */
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

const SENSOR_UUID = '0000fff0-0000-1000-8000-00805f9b34fb';
const SENSOR_CHARACTERISTIC = '0000fff1-0000-1000-8000-00805f9b34fb';

const bleManager = new BleManager();

type VoidCallback = (result: boolean) => void;

interface SensorData {
    speed: number | null;
    avgSpeed: number | null;
    distance: number | null;
    startTime: number | null;
    elapsedTime: number;
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
    startStopRun: (deviceId: Device) => Promise<void>;
    formatElapsedTime: (ms: number) => string;
    loadLastSensorData: () => Promise<SensorData | null>;
    loadLastConnectedDevice: () => Promise<{ id: string; name: string } | null>;
    saveSensorData: (data: SensorData) => Promise<void>;
}

const INVALID_U32 = 0xffffffff;

function base64ToUint8Array(base64: string): Uint8Array {
    const binaryString = atob(base64);
    const len = binaryString.length;
    const bytes = new Uint8Array(len);
    for (let i = 0; i < len; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return bytes;
}

function parseSensorReading(buffer: ArrayBuffer, offset: number): SensorData | null {
    const view = new DataView(buffer, offset, 32);

    const longitudeRaw = view.getUint32(0, true);
    const latitudeRaw = view.getUint32(4, true);
    const timestamp = view.getBigUint64(8, true);

    const longitude = decodeLongitude(longitudeRaw);
    const latitude = decodeLatitude(latitudeRaw);

    const acc_x = view.getInt8(16);
    const acc_y = view.getInt8(17);
    const acc_z = view.getInt8(18);

    const gyro_x = view.getInt16(20, true);
    const gyro_y = view.getInt16(22, true);
    const gyro_z = view.getInt16(24, true);

    const humidity = view.getUint8(26);
    const temperature = view.getInt8(27);

    const lastUpdatedAt = new Date();

    console.log('latitude', latitude);
    console.log('longitude', longitude);

    return {
        speed: null, // TODO: calculate speed if needed
        avgSpeed: null,
        distance: null,
        movingTime: null,
        temperature,
        humidity,
        location: { latitude, longitude },
        imu: { acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z },
        timestamp: Number(timestamp),
        lastUpdatedAt,
        isRunning: false,
    };
}

function decodeLongitude(raw: number): number | null {
    if (raw === INVALID_U32) return null;
    const SIGN_BIT = 0x80000000;
    const VALUE_MASK = 0x7fffffff;
    const COORDINATE_SCALE = 1e7;

    const isWest = (raw & SIGN_BIT) !== 0;
    const magnitude = raw & VALUE_MASK;
    const value = magnitude / COORDINATE_SCALE;
    return isWest ? -value : value;
}

function decodeLatitude(raw: number): number | null {
    if (raw === INVALID_U32) return null;
    const SIGN_BIT = 0x80000000;
    const VALUE_MASK = 0x7fffffff;
    const COORDINATE_SCALE = 1e7;

    const isSouth = (raw & SIGN_BIT) !== 0;
    const magnitude = raw & VALUE_MASK;
    const value = magnitude / COORDINATE_SCALE;
    return isSouth ? -value : value;
}

function useBLE(): BluetoothLowEnergyApi {
    const [allDevices, setAllDevices] = useState<Device[]>([]);
    const [connectedDevice, setConnectedDevice] = useState<Device | null>(null);
    const [sensorData, setSensorData] = useState<SensorData>({
        speed: null,
        avgSpeed: null,
        distance: null,
        startTime: null,
        elapsedTime: 0,
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
                console.log('device found');
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
            console.log('connecting');
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

    const onConnectedSensorsUpdate = useCallback((error: BleError | null, characteristic: Characteristic | null) => {
        if (error) {
            console.log(error);
            return;
        }
        if (!characteristic?.value) {
            console.log('no data received');
            return;
        }

        const bytes = base64ToUint8Array(characteristic.value);

        if (bytes.every(b => b === 0)) {
            return; // skips empty data bytes
        }

        console.log('data received');

        if (bytes.length < 32 || bytes.length % 32 !== 0) {
            console.warn('Invalid data length:', bytes.length);
            return;
        }

        function toArrayBuffer(buffer: ArrayBuffer | SharedArrayBuffer): ArrayBuffer {
            if (buffer instanceof ArrayBuffer) {
                return buffer;
            }
            const copy = new ArrayBuffer(buffer.byteLength);
            new Uint8Array(copy).set(new Uint8Array(buffer));
            return copy;
        }

        const buffer = toArrayBuffer(bytes.buffer);
        const numberOfReadings = bytes.length / 32;
        const readings: SensorData[] = [];

        for (let i = 0; i < numberOfReadings; i++) {
            const reading = parseSensorReading(buffer, i * 32);
            if (reading) {
                readings.push(reading);
            }
        }

        console.log('READINGS', readings);

        if (readings.length > 0) {
            setSensorData(readings[0]); // update with first valid reading
            saveSensorData(readings[0]); // saves locally
        }
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
                };
            } else {
                return {
                    ...prev,
                    isRunning: false,
                    startTime: null,
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
            return jsonValue != null ? JSON.parse(jsonValue) : null;
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
        startStopRun,
        formatElapsedTime,
        loadLastSensorData,
        loadLastConnectedDevice,
        saveSensorData,
    };
}

export default useBLE;
