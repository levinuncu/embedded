import React, { useState } from 'react';
import { StyleSheet, Button, View, Text, Alert, Platform, TouchableOpacity } from 'react-native';
import { SafeAreaView, SafeAreaProvider } from 'react-native-safe-area-context';
import DeviceModal from './DeviceConnectionModal';
import useBLE from './useBLE';

function showAlert(message: string | undefined) {
  Alert.alert('Connection', message)
}


const App = () => {
  const {
    requestPermissions,
    scanForPeripherals,
    allDevices,
    connectToDevice,
    connectedDevice,
    disconnectFromDevice,
  } = useBLE();

  const scanForDevices = () => {
    requestPermissions((isGranted: any) => {
      if (isGranted) {
        scanForPeripherals();
      }
    });
  };

  const [isModalVisible, setIsModalVisible] = useState<boolean>(false);
  const hideModal = async () => {
    setIsModalVisible(false);
  }

  const openModal = async () => {
    scanForDevices();
    setIsModalVisible(true);
  }
  return (
    <SafeAreaProvider style={styles.provider}>
      <Text style={styles.headerTitle}>
        Piezo Fly App
      </Text>
      <SafeAreaView style={styles.container}>
        <View>
          {connectedDevice ? (
            <Text style={styles.text}> Your connected Device is: {connectedDevice.localName}</Text>
          ) : (
            <Text style={styles.text}>Connect to a sensor</Text>
          )}
        </View>
        <TouchableOpacity style={styles.connectButton}
          onPress={connectedDevice ? disconnectFromDevice : openModal}>
          <Text style={styles.buttonText}>{connectedDevice ? 'Disconnect' : 'Connect'}</Text>
        </TouchableOpacity>
        <DeviceModal
          closeModal={hideModal}
          visible={isModalVisible}
          connectToPeripheral={connectToDevice}
          devices={allDevices} />
      </SafeAreaView>
    </SafeAreaProvider >
  );
}

const styles = StyleSheet.create({
  provider: {
    flex: 1,
    backgroundColor: '#f2f2f2',
  },
  headerTitle: {
    alignItems: 'flex-start',
    fontSize: 20,
    fontWeight: 'bold',
    marginTop: 50,
    marginLeft: 20,
  },
  container: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
  },
  text: {
    textAlign: 'center',
    marginVertical: 8,
    marginHorizontal: 20,
  },
  buttonText: {
    fontSize: 14,
    fontWeight: 'bold',
    color: 'white',
  },
  connectButton: {
    backgroundColor: '#c9094fcb',
    justifyContent: 'center',
    alignItems: 'center',
    height: 35,
    marginHorizontal: 20,
    marginBottom: 5,
    marginTop: 5,
    borderRadius: 8,
    paddingLeft: 20,
    paddingRight: 20,
  },
});

export default App;