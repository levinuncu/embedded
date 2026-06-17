import React, { useEffect, useState } from 'react';
import { StyleSheet, View, Text, Alert, TouchableOpacity, Dimensions } from 'react-native';
import { SafeAreaProvider } from 'react-native-safe-area-context';
import DeviceModal from './DeviceConnectionModal';
import useBLE from './useBLE';
import MapView, { Marker, PROVIDER_GOOGLE } from 'react-native-maps';
import AsyncStorage from '@react-native-async-storage/async-storage';

const screenWidth = Dimensions.get('window').width;
const mapWidth = screenWidth * 0.9;

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
    setSensorData,
    startStopRun,
    formatElapsedTime,
    loadLastSensorData,
    loadLastConnectedDevice,
    saveSensorData,
  } = useBLE();

  useEffect(() => {
    const initialize = async () => {
      const savedSensorData = await loadLastSensorData();
      if (savedSensorData) {
        setSensorData(savedSensorData);
      }
      const savedDevice = await loadLastConnectedDevice();
      if (savedDevice?.id) {
        reconnectToDevice(savedDevice.id);
      }
    };
    initialize();
  }, []);

  useEffect(() => {
    saveSensorData(sensorData);
  }, [sensorData]);

  const scanForDevices = () => {
    requestPermissions((isGranted: any) => {
      if (isGranted) {
        scanForPeripherals();
      }
    });
  };


  useEffect(() => {
    let interval: ReturnType<typeof setTimeout> | null = null;
    if (sensorData.isRunning && sensorData.startTime) {
      interval = setInterval(() => {
        setSensorData(prev => {
          if (!prev.isRunning || !prev.startTime) return prev;
          return {
            ...prev,
            elapsedTime: Date.now() - prev.startTime,
          };
        });
      }, 1000);
    }

    return () => {
      if (interval) clearInterval(interval);
    };
  }, [sensorData.isRunning, sensorData.startTime]);

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
    latitudeDelta: 0.01,
    longitudeDelta: 0.01,
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
        Piezo Fly
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
        {/* {connectedDevice ? ( */}
        <View>
          <View style={styles.container}>
            <Text style={styles.text}>Location data: </Text>
            {connectedDevice ? (<Text style={styles.lowOpacityText}>{sensorData?.location?.latitude} {sensorData?.location?.longitude}</Text>
            ) : (
              <Text style={styles.lowOpacityText}>49.122044, 9.211371</Text>
            )}
          </View>
          <View style={styles.flexContainer}>
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
              <Text style={styles.text}>Moving time: </Text>
              {connectedDevice ? (
                <Text style={styles.text}>
                  {connectedDevice ? formatElapsedTime(sensorData.elapsedTime) : '00:00'}
                </Text>
              ) : (
                <Text style={styles.text}>50min</Text>
              )}
              {/* {needs to retrieve the time in hh:mm
              from when start was clicked until stop was pressed - in between 
              it should retrieve the current amount of time that has passed since the start button press} */}
            </View>
            <View style={styles.container}>
              <TouchableOpacity style={styles.reconnectButton}
                onPress={() => startStopRun(connectedDevice)}
              >
                <Text style={styles.buttonText}>{sensorData.isRunning == false ? ('Start') : ('Stop')}</Text>
              </TouchableOpacity>
            </View>
          </View>
          <View style={styles.container}>
            <View style={styles.container}>
              <Text style={styles.text}>Distance: </Text>
              {connectedDevice ? (
                <Text style={styles.text}>{sensorData.distance ?? '--'}km</Text>
              ) : (
                <Text style={styles.text}>9km</Text>
              )}
              {/* {should stay empty until stop button was pressed and then should retrieve the covered distance in km 
              OR
              should retrieve the current covered distance without waiting for the stop button to be pressed} */}
            </View>
            <View style={styles.container}>
              <Text style={styles.text}>Average speed: </Text>
              {connectedDevice ? (
                <Text style={styles.text}>{sensorData.avgSpeed ?? '--'}/km</Text>
              ) : (
                <Text style={styles.text}>5:78/km</Text>

              )}
              {/* {should stay empty until stop button was pressed and then should
              calculate the average speed in mm:ss per km, e.g.: 5:37 / km } */}
            </View>
          </View>
          <View style={styles.container}>
            <View style={styles.container}>
              <Text style={styles.text}>Temperature: </Text>
              {connectedDevice ? (
                <Text style={styles.text}>{sensorData.temperature ?? '--'}°</Text>
              ) : (
                <Text style={styles.text}>29°</Text>
              )}

            </View>
            <View style={styles.container}>
              <Text style={styles.text}>Humidity: </Text>
              {connectedDevice ? (
                <Text style={styles.text}>{sensorData.humidity ?? '--'}%</Text>
              ) : (
                <Text style={styles.text}>20%</Text>
              )}

            </View>
          </View>
          <View style={styles.container}>
            <Text style={styles.lowOpacityText}>Last updated: </Text>
            {connectedDevice ? (
              <Text style={styles.lowOpacityText}>
                {last?.getHours()}:{last?.getUTCMinutes()} - {last?.getUTCDate()}.{last?.getMonth()}.{last?.getUTCFullYear()}
              </Text>
            ) : (
              <Text style={styles.lowOpacityText}>13:23 - 19.06.2026</Text>

            )}

            <TouchableOpacity style={styles.reconnectButton}
              onPress={() => reconnectToDevice(connectedDevice)}
            >
              <Text style={styles.buttonText}>Reload</Text>
            </TouchableOpacity>
          </View>
        </View>
        {/* ) : (
          <Text style={styles.lowOpacityText}>Connect to a sensor to see data</Text>
        )} */}
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
    flex: 1,
    flexDirection: 'row',
    marginHorizontal: 20,
    marginBottom: '50%',
    maxWidth: mapWidth,
    justifyContent: 'space-evenly',
  },
  flexContainer: {
    flex: 1,
  },
  map: {
    flex: 1,
    width: mapWidth,
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
    opacity: 0.2,
  },
  buttonText: {
    fontSize: 16,
    fontWeight: 'bold',
    color: 'white',
  },
  connectButton: {
    backgroundColor: 'rgb(140,190,7)',
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
    backgroundColor: 'rgb(140,190,7)',
    justifyContent: 'center',
    alignItems: 'center',
    height: 35,
    marginLeft: 20,
    marginBottom: 5,
    marginTop: 5,
    borderRadius: 8,
    paddingLeft: 20,
    paddingRight: 20,
  },
});

export default App;