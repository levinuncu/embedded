import React, { FC, useCallback } from 'react';
import {
    FlatList,
    Modal,
    Text,
    StyleSheet,
    TouchableOpacity,
    ListRenderItemInfo,
    View,
} from 'react-native';
import { Device } from 'react-native-ble-plx';

// declares type of a single list item
type DeviceModalListItemProps = {
    item: ListRenderItemInfo<Device>;
    connectToPeripheral: (device: Device) => void;
    closeModal: () => void;
};

// declares type of the whole list
type DeviceModalProps = {
    devices: Device[];
    visible: boolean;
    connectToPeripheral: (device: Device) => void;
    closeModal: () => void;
};

// returns a singular item for the list
const DeviceModalListItem: FC<DeviceModalListItemProps> = props => {
    const { item, connectToPeripheral, closeModal } = props;

    // connects to clicked device and closes modal
    const connectAndCloseModal = useCallback(() => {
        connectToPeripheral(item.item);
        closeModal();
    }, [closeModal, connectToPeripheral, item.item]);

    // returns a singular button for a singular device with the device name on it
    return (
        <TouchableOpacity
            onPress={connectAndCloseModal}
            style={styles.ctaButton}>
            <Text style={styles.ctaButtonText}>{item.item.name}</Text>
        </TouchableOpacity>
    );
};

const DeviceModal: FC<DeviceModalProps> = props => {
    const { devices, visible, connectToPeripheral, closeModal } = props;

    // renders devices to be a list item
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

    // returns a modal that lists up all of the available devices
    return (
        <Modal
            style={styles.modalContainer}
            animationType="slide"
            transparent={false}
            visible={visible}>
            <View style={styles.modalTitle}>
                {/* button to exit the modal */}
                <TouchableOpacity style={styles.closeButton}
                    onPress={closeModal}>
                    <Text style={styles.buttonText}>x</Text>
                </TouchableOpacity>
                {/* title of modal */}
                <Text style={styles.modalTitleText}>
                    Tap on a device to connect
                </Text>
                {/* list with items, a.k.a. devices to connect to */}
                <FlatList
                    contentContainerStyle={styles.modalFlatlistContiner}
                    data={devices}
                    renderItem={renderDeviceModalListItem}
                />
            </View>
        </Modal>
    );
};

// css stylesheet for UI 
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
        fontSize: 24,
        fontWeight: 'bold',
        marginHorizontal: 20,
        textAlign: 'left',
    },
    buttonText: {
        fontSize: 20,
        fontWeight: 'bold',
        fontFamily: 'copperplate',
    },
    closeButton: {
        justifyContent: 'flex-end',
        alignItems: 'flex-end',
        height: 20,
        marginTop: 50,
        marginHorizontal: 20,
    },
    ctaButton: {
        backgroundColor: 'rgb(140,190,7)',
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