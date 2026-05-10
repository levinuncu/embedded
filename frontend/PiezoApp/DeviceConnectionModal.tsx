import React, { FC, useCallback } from 'react';
import {
    FlatList,
    Modal,
    SafeAreaView,
    Text,
    StyleSheet,
    TouchableOpacity,
    ListRenderItemInfo,
} from 'react-native';
import { Base64, BleError, ConnectionPriority, Descriptor, Device, Subscription } from 'react-native-ble-plx';

type DeviceModalListItemProps = {
    item: ListRenderItemInfo<Device>;
    connectToPeripheral: (device: Device) => void;
    closeModal: () => void;
};

type DeviceModalProps = {
    devices: Device[];
    visible: boolean;
    connectToPeripheral: (device: Device) => void;
    closeModal: () => void;
};

const DeviceModalListItem: FC<DeviceModalListItemProps> = props => {
    const { item, connectToPeripheral, closeModal } = props;

    const connectAndCloseModal = useCallback(() => {
        connectToPeripheral(item.item);
        closeModal();
    }, [closeModal, connectToPeripheral, item.item]);

    return (
        <TouchableOpacity
            onPress={connectAndCloseModal}
            style={styles.ctaButton}>
            <Text style={styles.ctaButtonText}>{item.item.name}</Text>
        </TouchableOpacity>
    );
};

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
        id: "12:34:56:78:9A:BC",
        name: "ESP-8",
        localName: "ESP-8",
        rssi: -55,
        serviceUUIDs: ["180D"],
        isConnectable: true,
    }),
    createMockDevice({
        id: "98:76:54:32:10:FE",
        name: "ESP-16",
        localName: "ESP-16",
        rssi: -70,
        serviceUUIDs: ["1809"],
        isConnectable: true,
    }),
    createMockDevice({
        id: "AB:CD:EF:12:34:56",
        name: "ESP-32",
        localName: "ESP-32",
        rssi: -60,
        serviceUUIDs: ["181A"],
        isConnectable: false,
    }),
    createMockDevice({
        id: "01:23:45:67:89:AB",
        name: "Unkown",
        localName: "UnknownDevice",
        rssi: -80,
        serviceUUIDs: [],
        isConnectable: false,
    }),
];

const DeviceModal: FC<DeviceModalProps> = props => {
    const { devices, visible, connectToPeripheral, closeModal } = props;

    const renderDeviceModalListItem = useCallback(
        (item: ListRenderItemInfo<Device>) => {
            return (
                <DeviceModalListItem
                    item={item}
                    connectToPeripheral={connectToPeripheral}
                    closeModal={closeModal}
                />
            );
        },
        [closeModal, connectToPeripheral],
    );

    return (
        <Modal
            style={styles.modalContainer}
            animationType="slide"
            transparent={false}
            visible={visible}>
            <SafeAreaView style={styles.modalTitle}>
                <TouchableOpacity style={styles.closeButton}
                    onPress={closeModal}>
                    <Text style={styles.buttonText}>x</Text>
                </TouchableOpacity>
                <Text style={styles.modalTitleText}>
                    Tap on a device to connect
                </Text>
                <FlatList
                    contentContainerStyle={styles.modalFlatlistContiner}
                    data={mockDevices}
                    renderItem={renderDeviceModalListItem}
                />
            </SafeAreaView>
        </Modal>
    );
};

const styles = StyleSheet.create({
    modalContainer: {
        flex: 1,
        backgroundColor: '#f2f2f2',
    },
    modalFlatlistContiner: {
        flex: 1,
        justifyContent: 'flex-start',
    },
    modalCellOutline: {
        borderWidth: 1,
        borderColor: 'black',
        alignItems: 'center',
        marginHorizontal: 20,
        paddingVertical: 15,
        borderRadius: 8,
    },
    modalTitle: {
        flex: 1,
        backgroundColor: '#f2f2f2',
    },
    modalTitleText: {
        justifyContent: 'flex-start',
        marginBottom: 20,
        fontSize: 18,
        fontWeight: 'bold',
        marginHorizontal: 20,
        textAlign: 'left',
    },
    buttonText: {
        fontSize: 22,
        fontWeight: 'bold',
        fontFamily: 'copperplate',
    },
    closeButton: {
        justifyContent: 'flex-end',
        alignItems: 'flex-end',
        marginTop: 50,
        marginHorizontal: 20,
    },
    ctaButton: {
        backgroundColor: '#c9094fcb',
        justifyContent: 'center',
        alignItems: 'center',
        height: 40,
        marginHorizontal: 20,
        marginBottom: 5,
        borderRadius: 8,
    },
    ctaButtonText: {
        fontSize: 16,
        fontWeight: 'bold',
        color: 'white',
    },
});

export default DeviceModal;