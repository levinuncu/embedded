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

import { atob } from 'react-native-quick-base64';


const createMockDevice = (overrides: Partial<Device>): Device => {
    const device: Device = {
        id: "00:00:00:00:00:00",
        name: null,
        localName: null,
        rssi: null,
        mtu: 23,
        rawScanRecord: "",
        serviceUUIDs: null,
        solicitedServiceUUIDs: null,
        overflowServiceUUIDs: null,
        manufacturerData: null,
        serviceData: null,
        txPowerLevel: null,
        isConnectable: null,

        requestConnectionPriority: (priority: ConnectionPriority) => Promise.resolve(device),
        readRSSI: () => Promise.resolve(device),
        requestMTU: (mtu: number) => Promise.resolve(device),
        connect: (options?: any) => Promise.resolve(device),
        cancelConnection: () => Promise.resolve(device),
        isConnected: () => Promise.resolve(true),
        discoverAllServicesAndCharacteristics: () => Promise.resolve(device),
        onDisconnected: (listener: (error: BleError | null, device: Device) => void): Subscription => {
            return { remove: () => { } };
        },

        services: () => Promise.resolve([]),
        characteristicsForService: (uuid: string) => Promise.resolve([]),
        descriptorsForService: (sUuid: string, cUuid: string) => Promise.resolve([]),
        readCharacteristicForService: (sUuid: string, cUuid: string) => Promise.resolve({} as any),
        writeCharacteristicWithResponseForService: (sUuid: string, cUuid: string, value: Base64) => Promise.resolve({} as any),
        writeCharacteristicWithoutResponseForService: (sUuid: string, cUuid: string, value: Base64) => Promise.resolve({} as any),
        monitorCharacteristicForService: (sUuid: string, cUuid: string, cb: any) => ({ remove: () => { } }),

        readDescriptorForService: (serviceUUID: string, characteristicUUID: string, descriptorUUID: string, transactionId?: string) => {
            return Promise.resolve({} as Descriptor);
        },

        writeDescriptorForService: (serviceUUID: string, characteristicUUID: string, descriptorUUID: string, valueBase64: Base64, transactionId?: string) => {
            return Promise.resolve({} as Descriptor);
        },
        ...overrides,
    };

    return device;
};

const mockDevices: Device[] = [
    createMockDevice({
        id: "AB:CD:EF:12:34:56",
        name: "ESP-32",
        localName: "ESP-32",
        rssi: -60,
        serviceUUIDs: ["181A"],
        isConnectable: false,
    }),
];

const SENSOR_UUID = '0000180d-0000-1000-8000-00805f9b34fb';
const SENSOR_CHARACTERISTIC = '00002a37-0000-1000-8000-00805f9b34fb';

const bleManager = new BleManager();

type VoidCallback = (result: boolean) => void;

interface BluetoothLowEnergyApi {
    requestPermissions(cb: VoidCallback): Promise<void>;
    scanForPeripherals(): void;
    connectToDevice: (deviceId: Device) => Promise<void>;
    disconnectFromDevice: () => void;
    connectedDevice: Device | null;
    allDevices: Device[];
    connectedSensors: number;
}

function useBLE(): BluetoothLowEnergyApi {
    const [allDevices, setAllDevices] = useState<Device[]>([]);
    const [connectedDevice, setConnectedDevice] = useState<Device | null>(null);
    const [connectedSensors, setConnectedSensors] = useState<number>(0);

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
            if (device && device.name?.includes('ESP')) {
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
            bleManager.stopDeviceScan();
            startStreamingData(deviceConnection);
        } catch (e) {
            // console.log('FAILED TO CONNECT', e);
            setConnectedDevice(mockDevices[0]);
            console.log('connection mocked with ' + mockDevices[0].name);
        }
    };

    const disconnectFromDevice = () => {
        if (connectedDevice) {
            bleManager.cancelDeviceConnection(connectedDevice.id);
            setConnectedDevice(null);
            console.log('disconnected mock connection');
        }
    };

    const onConnectedSensorsUpdate = (
        error: BleError | null,
        characteristic: Characteristic | null,
    ) => {
        if (error) {
            console.log(error);
            return -1;
        } else if (!characteristic?.value) {
            console.log('No Data was recieved');
            return -1;
        }

        const rawData = atob(characteristic.value);
        let innerConnectedSensors: number = -1;

        const firstBitValue: number = Number(rawData) & 0x01;

        if (firstBitValue === 0) {
            innerConnectedSensors = rawData[1].charCodeAt(0);
        } else {
            innerConnectedSensors =
                Number(rawData[1].charCodeAt(0) << 8) +
                Number(rawData[2].charCodeAt(2));
        }

        setConnectedSensors(innerConnectedSensors);
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
    };
}

export default useBLE;