import React, { useEffect, useState } from 'react';
import { StyleSheet, View, Text, Alert, TouchableOpacity } from 'react-native';
import { SafeAreaProvider } from 'react-native-safe-area-context';
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
    reconnectToDevice,
    connectedDevice,
    disconnectFromDevice,
    sensorData,
  } = useBLE();

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

  const [region, setRegion] = useState({
    // HHN location
    latitude: 49.122044,
    longitude: 9.211371,
    // for zoom:
    latitudeDelta: 0.001,
    longitudeDelta: 0.001,
  });

  useEffect(() => {
    if (
      sensorData.location &&
      sensorData.location.latitude !== null &&
      sensorData.location.longitude !== null
    ) {
      setRegion({
        latitude: sensorData.location.latitude,
        longitude: sensorData.location.longitude,
        latitudeDelta: 0.001,
        longitudeDelta: 0.001,
      });
    }
  }, [sensorData.location]);

  const last = sensorData.lastUpdatedAt;

  return (
    <SafeAreaProvider style={styles.provider}>
      <Text style={styles.headerTitle}>
        Piezo Fly App
      </Text>
      <View style={styles.container}>
        <View>
          {connectedDevice ? (
            <Text style={styles.textAbove}>Your connected Device is: {connectedDevice.name}</Text>
          ) : (
            <Text style={styles.textAbove}>Connect to a sensor</Text>
          )}
        </View>
        <TouchableOpacity style={styles.connectButton}
          onPress={connectedDevice ? disconnectFromDevice : openModal}>
          <Text style={styles.buttonText}>{connectedDevice ? 'Disconnect' : 'Connect'}</Text>
        </TouchableOpacity>
      </View>
      <View style={styles.dataContainer}>
        {connectedDevice ? (
          <View>
            <View style={styles.container}>
              <Text style={styles.text}>Location data: </Text>
              <Text style={styles.lowOpacityText}>{sensorData?.location?.latitude} {sensorData?.location?.longitude}</Text>
            </View>
            <View>
              <MapView
                provider={PROVIDER_GOOGLE}
                style={styles.map}
                region={region}
                onRegionChangeComplete={setRegion}
              >
                {sensorData.location &&
                  sensorData.location.latitude !== null &&
                  sensorData.location.longitude !== null && (
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
            <View style={styles.container}>
              <Text style={styles.lowOpacityText}>Last updated: </Text>
              <Text style={styles.lowOpacityText}>{last?.getUTCHours()}:{last?.getUTCMinutes()} - {last?.getUTCDate()}.{last?.getUTCMonth()}.{last?.getUTCFullYear()}</Text>
              <TouchableOpacity style={styles.reconnectButton}
                onPress={() => reconnectToDevice(connectedDevice)}
              >
                <Text style={styles.buttonText}>Reload</Text>
              </TouchableOpacity>
            </View>
          </View>
        ) : (
          <Text style={styles.lowOpacityText}>Connect to a sensor to see data</Text>
        )}
      </View>

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
    fontSize: 24,
    fontWeight: 'bold',
    marginTop: 50,
    marginLeft: 20,
  },
  container: {
    flexDirection: 'row',
    alignItems: 'center',
    marginVertical: 5,
    justifyContent: 'space-between',
  },
  dataContainer: {
    flexDirection: 'row',
    marginHorizontal: 20,
    justifyContent: 'space-evenly'
  },
  map: {
    width: '100%',
    height: '60%',
  },
  textAbove: {
    textAlign: 'center',
    fontSize: 16,
    marginVertical: 8,
    marginHorizontal: 20,
  },
  text: {
    textAlign: 'center',
    fontSize: 16,
    marginVertical: 8,
  },
  lowOpacityText: {
    textAlign: 'center',
    opacity: 0.2,
  },
  buttonText: {
    fontSize: 16,
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
  reconnectButton: {
    backgroundColor: '#c9094fcb',
    justifyContent: 'center',
    alignItems: 'center',
    height: 35,
    marginLeft: 130,
    marginBottom: 5,
    marginTop: 5,
    borderRadius: 8,
    paddingLeft: 20,
    paddingRight: 20,
  },
});

export default App;