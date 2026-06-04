/* eslint-disable no-bitwise */
import { useState } from 'react';
import { PermissionsAndroid, Platform } from 'react-native';
import {
    Base64,
    BleError,
    BleManager,
    Characteristic,
    ConnectionPriority,
    Descriptor,
    Device,
    Subscription,
} from 'react-native-ble-plx';
import { PERMISSIONS, requestMultiple } from 'react-native-permissions';
import DeviceInfo from 'react-native-device-info';

const SENSOR_UUID = '0000180d-0000-1000-8000-00805f9b34fb';
const SENSOR_CHARACTERISTIC = '00002a37-0000-1000-8000-00805f9b34fb';

const bleManager = new BleManager();

type VoidCallback = (result: boolean) => void;

interface SensorData {
    speed: number | null;
    temperature: number | null;
    humidity: number | null;
    location: { latitude: number; longitude: number } | null;
    imu: {
        acc_x: number;
        acc_y: number;
        acc_z: number;
        gyro_x: number;
        gyro_y: number;
        gyro_z: number;
    } | null;
    timestamp: number | null;
}


interface BluetoothLowEnergyApi {
    requestPermissions(cb: VoidCallback): Promise<void>;
    scanForPeripherals(): void;
    connectToDevice: (deviceId: Device) => Promise<void>;
    disconnectFromDevice: () => void;
    connectedDevice: Device | null;
    allDevices: Device[];
    connectedSensors: number;
    sensorData: SensorData;
}

function useBLE(): BluetoothLowEnergyApi {
    const [allDevices, setAllDevices] = useState<Device[]>([]);
    const [connectedDevice, setConnectedDevice] = useState<Device | null>(null);
    const [connectedSensors, setConnectedSensors] = useState<number>(0);
    const [sensorData, setSensorData] = useState({
        speed: 5,
        temperature: 0,
        humidity: 0,
        location: {
            latitude: 0,
            longitude: 0,
        },
        imu: {
            acc_x: 0,
            acc_y: 0,
            acc_z: 0,
            gyro_x: 0,
            gyro_y: 0,
            gyro_z: 0,
        },
        timestamp: 0,
    });

    const requestPermissions = async (cb: VoidCallback) => {
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
                    result['android.permission.BLUETOOTH_CONNECT'] ===
                    PermissionsAndroid.RESULTS.GRANTED &&
                    result['android.permission.BLUETOOTH_SCAN'] ===
                    PermissionsAndroid.RESULTS.GRANTED &&
                    result['android.permission.ACCESS_FINE_LOCATION'] ===
                    PermissionsAndroid.RESULTS.GRANTED;

                cb(isGranted);
            }
        } else {
            cb(true);
        }
    };

    const isDuplicteDevice = (devices: Device[], nextDevice: Device) =>
        devices.findIndex(device => nextDevice.id === device.id) > -1;

    const scanForPeripherals = () =>
        bleManager.startDeviceScan(null, null, (error, device) => {
            if (error) {
                console.log(error);
            }
            if (device && device.name?.includes('piezo')) {
                console.log('DEVICE FOUND');
                setAllDevices((prevState: Device[]) => {
                    if (!isDuplicteDevice(prevState, device)) {
                        return [...prevState, device];
                    }
                    return prevState;
                });
            }
        });

    const connectToDevice = async (device: Device) => {
        try {
            const deviceConnection = await bleManager.connectToDevice(device.id);
            setConnectedDevice(deviceConnection);
            await deviceConnection.discoverAllServicesAndCharacteristics();
            stopScanningForDevices();
            startStreamingData(deviceConnection);
        } catch (e) {
            console.log('FAILED TO CONNECT', e);
        }
    };

    const stopScanningForDevices = () =>
        bleManager.stopDeviceScan();

    const disconnectFromDevice = () => {
        if (connectedDevice) {
            bleManager.cancelDeviceConnection(connectedDevice.id);
            setConnectedDevice(null);
            console.log('disconnected: ', connectedDevice.name);
        }
    };

    const onConnectedSensorsUpdate = (error: BleError | null, characteristic: Characteristic | null) => {
        if (error) {
            console.log(error);
            return;
        }
        if (!characteristic?.value) {
            console.log('No Data was received');
            return;
        }

        // decodes base64 to ArrayBuffer
        const base64 = characteristic.value;
        const binaryString = atob(base64);
        const len = binaryString.length;
        const bytes = new Uint8Array(len);
        for (let i = 0; i < len; i++) {
            bytes[i] = binaryString.charCodeAt(i);
        }

        if (bytes.length < 32 || bytes.length % 32 !== 0) {
            console.warn('Invalid data length:', bytes.length);
            return;
        }

        const numberOfReadings = bytes.length / 32;
        const readings = [];

        for (let i = 0; i < numberOfReadings; i++) {
            const offset = i * 32;
            const view = new DataView(bytes.buffer, offset, 32);

            // GNSS
            const longitude = view.getUint32(0, true); // little endian
            const latitude = view.getUint32(4, true);
            const timestampLow = view.getUint32(8, true);
            const timestampHigh = view.getUint32(12, true);
            // combines low and high to form 64-bit timestamp
            const timestamp = timestampHigh * 2 ** 32 + timestampLow;

            // IMU
            const acc_x = view.getInt8(16);
            const acc_y = view.getInt8(17);
            const acc_z = view.getInt8(18);
            // skips padding at 19
            const gyro_x = view.getInt16(20, true);
            const gyro_y = view.getInt16(22, true);
            const gyro_z = view.getInt16(24, true);

            // temp
            const humidity = view.getUint8(26);
            const temperature = view.getInt8(27);

            readings.push({
                longitude,
                latitude,
                timestamp,
                acc_x,
                acc_y,
                acc_z,
                gyro_x,
                gyro_y,
                gyro_z,
                humidity,
                temperature,
            });
        }

        // takes the first reading and updates sensorData state
        if (readings.length > 0) {
            const first = readings[0];
            setSensorData({
                speed: 0, // TODO: calculate speed from GNSS
                temperature: first.temperature,
                humidity: first.humidity,
                location: {
                    latitude: first.latitude,
                    longitude: first.longitude,
                },
                imu: {
                    acc_x: first.acc_x,
                    acc_y: first.acc_y,
                    acc_z: first.acc_z,
                    gyro_x: first.gyro_x,
                    gyro_y: first.gyro_y,
                    gyro_z: first.gyro_z,
                },
                timestamp: first.timestamp,
            });
        }
    };

    const startStreamingData = async (device: Device) => {
        if (device) {
            device.monitorCharacteristicForService(
                SENSOR_UUID,
                SENSOR_CHARACTERISTIC,
                (error, characteristic) => onConnectedSensorsUpdate(error, characteristic),
            );
        } else {
            console.log('No Device Connected');
        }
    };

    return {
        scanForPeripherals,
        requestPermissions,
        connectToDevice,
        allDevices,
        connectedDevice,
        disconnectFromDevice,
        connectedSensors: connectedSensors,
        sensorData,
    };
}

export default useBLE;