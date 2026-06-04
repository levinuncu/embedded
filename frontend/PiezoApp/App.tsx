import React, { useEffect, useState } from 'react';
import { StyleSheet, Button, View, Text, Alert, Platform, TouchableOpacity } from 'react-native';
import { SafeAreaView, SafeAreaProvider } from 'react-native-safe-area-context';
import DeviceModal from './DeviceConnectionModal';
import useBLE from './useBLE';
import MapView, { Marker, PROVIDER_GOOGLE } from 'react-native-maps';

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
    sensorData,
  } = useBLE();

  const [region, setRegion] = useState({
    latitude: 0,
    longitude: 0,
    // for zoom:
    latitudeDelta: 0,
    longitudeDelta: 0,
  });

  const scanForDevices = () => {
    requestPermissions((isGranted: any) => {
      if (isGranted) {
        scanForPeripherals();
        console.log('scanning');
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

  useEffect(() => {
    if (sensorData.location) {
      setRegion({
        latitude: sensorData.location.latitude,
        longitude: sensorData.location.longitude,
        latitudeDelta: 0.01,
        longitudeDelta: 0.01,
      });
    }
  }, [sensorData.location]);

  return (
    <SafeAreaProvider style={styles.provider}>
      <Text style={styles.headerTitle}>
        Piezo Fly App
      </Text>
      <SafeAreaView style={styles.container}>
        <View>
          {connectedDevice ? (
            <Text style={styles.text}>Your connected Device is: {connectedDevice.name}</Text>
          ) : (
            <Text style={styles.text}>Connect to a sensor</Text>
          )}
        </View>
        <TouchableOpacity style={styles.connectButton}
          onPress={connectedDevice ? disconnectFromDevice : openModal}>
          <Text style={styles.buttonText}>{connectedDevice ? 'Disconnect' : 'Connect'}</Text>
        </TouchableOpacity>
      </SafeAreaView>
      <SafeAreaView style={styles.dataContainer}>
        <View>
          {connectedDevice ? (
            <View>
              <View style={styles.locationContainer}>
                <Text style={styles.text}>Location data: </Text>
                <MapView
                  provider={PROVIDER_GOOGLE}
                  style={styles.map}
                  region={region}
                  onRegionChangeComplete={setRegion}
                >
                  {sensorData.location && (
                    <Marker
                      coordinate={{
                        latitude: sensorData.location.latitude,
                        longitude: sensorData.location.longitude,
                      }}
                      title="Current Location"
                    />
                  )}
                </MapView>
              </View>
              <View style={styles.container}>
                <View style={styles.container}>
                  <Text style={styles.text}>Speed: </Text>
                  <Text style={styles.text}>{sensorData.speed ?? '--'}km/h</Text>
                </View>
              </View>
              <View style={styles.container}>
                <View style={styles.container}>
                  <Text style={styles.text}>Temperature: </Text>
                  <Text style={styles.text}>{sensorData.temperature ?? '--'}°</Text>
                </View>
                <View style={styles.container}>
                  <Text style={styles.text}>Humidity: </Text>
                  <Text style={styles.text}>{sensorData.humidity ?? '--'}%</Text>
                </View>
              </View>
            </View>
          ) : (
            <Text style={styles.placeholderText}>Connect to a sensor to see data</Text>
          )}
        </View>
      </SafeAreaView>

      <DeviceModal
        closeModal={hideModal}
        visible={isModalVisible}
        connectToPeripheral={connectToDevice}
        devices={allDevices} />
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
  dataContainer: {
    flexDirection: 'row',
    alignContent: 'space-between',
  },
  locationContainer: {
    flexDirection: 'column',
    alignItems: 'center',
    alignContent: 'space-between',
  },
  map: {
    width: '100%',
    height: '60%',
  },
  text: {
    textAlign: 'center',
    marginVertical: 8,
    marginHorizontal: 20,
  },
  placeholderText: {
    textAlign: 'center',
    marginVertical: 8,
    marginHorizontal: 20,
    opacity: 0.2,
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