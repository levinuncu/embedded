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

  const last = sensorData.lastUpdatedAt instanceof Date
    ? sensorData.lastUpdatedAt
    : sensorData.lastUpdatedAt
      ? new Date(sensorData.lastUpdatedAt)
      : null;

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
                <Text style={styles.text}>23:07</Text>
              )}
            </View>
            {connectedDevice ? (
              <View style={styles.container}>
                <TouchableOpacity style={styles.reconnectButton}
                  onPress={() => startStopRun(connectedDevice)}
                >
                  <Text style={styles.buttonText}>{sensorData.isRunning == false ? ('Start') : ('Stop')}</Text>
                </TouchableOpacity>
              </View>
            ) : (<Text></Text>)}
          </View>
          <View style={styles.container}>
            <View style={styles.container}>
              <Text style={styles.text}>Distance: </Text>
              {connectedDevice ? (
                <Text style={styles.text}>
                  {sensorData.isRunning
                    ? (sensorData.distance !== null ? sensorData.distance.toFixed(2) : '--') + ' km'
                    : (sensorData.distance !== null ? sensorData.distance.toFixed(2) : '0') + ' km'}
                </Text>
              ) : (
                <Text style={styles.text}>9 km</Text>
              )}
            </View>
            <View style={styles.container}>
              <Text style={styles.text}>Average speed: </Text>
              {connectedDevice ? (
                <Text style={styles.text}>
                  {sensorData.isRunning || sensorData.avgSpeed === null
                    ? '--'
                    : `${formatElapsedTime(sensorData.avgSpeed)}/km`}
                </Text>
              ) : (
                <Text style={styles.text}>5:78/km</Text>
              )}
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
            {connectedDevice && last ? (
              <Text style={styles.lowOpacityText}>
                {last.getHours().toString().padStart(2, '0')}:
                {last.getMinutes().toString().padStart(2, '0')} - {last.getDate()}.
                {last.getMonth() + 1}.{last.getFullYear()} // months usually start with 0 so add +1 for human comprehension
              </Text>
            ) : (
              <Text style={styles.lowOpacityText}>13:23 - 19.06.2026</Text>
            )}
            {connectedDevice ? (
              <TouchableOpacity
                style={styles.reconnectButton}
                onPress={() => reconnectToDevice(connectedDevice.id)}
              >
                <Text style={styles.buttonText}>Reload</Text>
              </TouchableOpacity>
            ) : (
              <Text></Text>
            )}
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