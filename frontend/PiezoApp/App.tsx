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
    <SafeAreaProvider>
      <SafeAreaView style={styles.container}>
        <View>
          {connectedDevice ? (
            <Text style={styles.title}> Your connected Device is: {connectedDevice.localName}</Text>
          ) : (
            <Text style={styles.title}>Connect to the sensor</Text>
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
  container: {
    flex: 1,
    backgroundColor: '#f2f2f2',
    alignItems: 'center',
    justifyContent: 'center',
    marginHorizontal: 16,
  },
  title: {
    textAlign: 'center',
    marginVertical: 8,
  },
  buttonText: {
    fontSize: 18,
    fontWeight: 'bold',
    color: 'white',
  },
  connectButton: {
    backgroundColor: 'purple',
    justifyContent: 'center',
    alignItems: 'center',
    height: 50,
    marginHorizontal: 20,
    marginBottom: 5,
    borderRadius: 8,
  },
});

export default App;