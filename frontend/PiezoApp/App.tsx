import React, { useEffect, useRef, useState } from 'react';
import { StyleSheet, View, Text, Alert, TouchableOpacity, Dimensions, ActivityIndicator } from 'react-native';
import { SafeAreaProvider } from 'react-native-safe-area-context';
import DeviceModal from './DeviceConnectionModal';
import useBLE from './useBLE';
import MapView, { Marker, PROVIDER_GOOGLE } from 'react-native-maps';
import SensorChart from './SensorChart';

// gets screen witdh to then calculate the maps width
const screenWidth = Dimensions.get('window').width;
const mapWidth = screenWidth * 0.9;

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
  const [isReconnecting, setIsReconnecting] = useState<boolean | null>(null);
  const [hasSavedDevice, setHasSavedDevice] = useState(false);
  const hasInitialized = useRef(false);

  // initializes
  useEffect(() => {
    const initialize = async () => {
      const savedSensorData = await loadLastSensorData();
      // gets prev data and loads it onto the UI
      if (savedSensorData) {
        setSensorData(prev => ({
          ...prev,
          ...savedSensorData,
          isRunning: false,
          startTime: null,
        }));
      }

      // gets prev device and tries reconnecting it
      const savedDevice = await loadLastConnectedDevice();
      if (savedDevice?.id) {
        setHasSavedDevice(true);
        setIsReconnecting(true);
        await reconnectToDevice(savedDevice.id);
        setIsReconnecting(false);
      } else {
        setIsReconnecting(false);
      }

      hasInitialized.current = true;
    };
    initialize();
  }, []);

  // once the initialization is over, the data should load in
  useEffect(() => {
    if (!hasInitialized.current) return;
    saveSensorData(sensorData);
  }, [sensorData]);

  // scans for devices after permissions have been granted
  const scanForDevices = () => {
    requestPermissions((isGranted: any) => {
      if (isGranted) {
        scanForPeripherals();
      }
    });
  };

  // increments the elapsed time in moving time continously
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

  // handles the modal for scanning devices
  const [isModalVisible, setIsModalVisible] = useState<boolean>(false);
  const hideModal = async () => {
    setIsModalVisible(false);
  }
  const openModal = async () => {
    scanForDevices();
    setIsModalVisible(true);
  }

  // saves the HHN location to set the region on the map
  const [region, setRegion] = useState({
    // HHN location
    latitude: 49.122044,
    longitude: 9.211371,
    // for zoom:
    latitudeDelta: 0.01,
    longitudeDelta: 0.01,
  });

  // checks for actual location data and returns that
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

  // gets the lastUpdatedAt parameter for the sensor data and saves it in a const to reduce code
  const last = sensorData.lastUpdatedAt instanceof Date
    ? sensorData.lastUpdatedAt
    : sensorData.lastUpdatedAt
      ? new Date(sensorData.lastUpdatedAt)
      : null;

  // shows a loading spinner for when hasn't been initialized yet
  if (isReconnecting === null) {
    return (
      <SafeAreaProvider style={styles.provider}>
        <ActivityIndicator style={styles.centered} size="large" color="rgb(140,190,7)" />
      </SafeAreaProvider>
    );
  }

  // returns actual UI elements
  return (
    <SafeAreaProvider style={styles.provider}>
      <Text style={styles.headerTitle}>
        Piezo Fly
      </Text>
      <View style={styles.container}>
        <View>
          {/* texts changing depending on the connection state */}
          {isReconnecting ? (
            <Text style={styles.textAbove}>Reconnecting to sensor...</Text>
          ) : connectedDevice ? (
            <Text style={styles.textAbove}>Your connected Device is: {connectedDevice.name}</Text>
          ) : (
            <Text style={styles.textAbove}>
              {hasSavedDevice ? 'Could not reconnect — connect manually' : 'Connect to a sensor'}
            </Text>
          )}
        </View>
        <TouchableOpacity
          style={[styles.connectButton, isReconnecting ? styles.connectButtonDisabled : null]}
          disabled={!!isReconnecting}
          onPress={connectedDevice ? disconnectFromDevice : openModal}
        >
          <Text style={styles.buttonText}>
            {isReconnecting ? '...' : connectedDevice ? 'Disconnect' : 'Connect'}
          </Text>
        </TouchableOpacity>
      </View>
      <View style={styles.dataContainer}>
        <View>
          <View style={styles.container}>
            <Text style={styles.text}>Location data: </Text>
          </View>
          <View style={styles.flexContainer}>
            <MapView
              provider={PROVIDER_GOOGLE}
              style={styles.map}
              region={region}
              onRegionChangeComplete={setRegion}
            >
              {/* checks for location data & displays it */}
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
              <Text style={styles.text}>
                {connectedDevice ? formatElapsedTime(sensorData.elapsedTime) : '00:00'}
              </Text>
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
              <Text style={styles.text}>
                {sensorData.isRunning
                  ? (sensorData.distance !== null ? sensorData.distance.toFixed(2) : '--') + ' km'
                  : (sensorData.distance !== null ? sensorData.distance.toFixed(2) : '0') + ' km'}
              </Text>
            </View>
            <View style={styles.container}>
              <Text style={styles.text}>Average speed: </Text>
              <Text style={styles.text}>
                {sensorData.isRunning || sensorData.avgSpeed === null
                  ? '--'
                  : `${formatElapsedTime(sensorData.avgSpeed)}/km`}
              </Text>
            </View>

          </View>
          <View style={styles.container}>
            <View style={styles.container}>
              <Text style={styles.text}>Temperature: </Text>
              <Text style={styles.text}>{sensorData.temperature ?? '--'}°C</Text>
            </View>
            <View style={styles.container}>
              <Text style={styles.text}>Humidity: </Text>
              <Text style={styles.text}>{sensorData.humidity ?? '--'}%</Text>
            </View>
            <View style={styles.container}>
              <Text style={styles.text}>Current: </Text>
              <Text style={styles.text}>{sensorData.current ?? '--'}A</Text>
            </View>
          </View>
          <View style={styles.container}>
            <View style={styles.container}>
              <Text style={styles.text}>Accaleration stats: </Text>
              <Text style={styles.text}>({sensorData.imu?.acc_x ?? '--'}, {sensorData.imu?.acc_y ?? '--'}, {sensorData.imu?.acc_z ?? '--'})</Text>
            </View>
            <View style={styles.container}>
              <Text style={styles.text}>Gyroscope stats: </Text>
              <Text style={styles.text}>({sensorData.imu?.gyro_x ?? '--'}, {sensorData.imu?.gyro_y ?? '--'}, {sensorData.imu?.gyro_z ?? '--'})</Text>
            </View>
          </View>
          {/* the sensor charts */}
          <SensorChart
            temperature={sensorData.temperature}
            humidity={sensorData.humidity}
            imu={sensorData.imu}
            timestamp={sensorData.lastUpdatedAt?.getTime() ?? null}
            isConnected={!!connectedDevice}
          />
          <View style={styles.container}>
            {/* retrieves the last time the UI has been updated */}
            <Text style={styles.lowOpacityText}>Last updated: </Text>
            {last ? (
              <Text style={styles.lowOpacityText}>
                {last.getHours().toString().padStart(2, '0')}:
                {last.getMinutes().toString().padStart(2, '0')} - {last.getDate()}.
                {last.getMonth() + 1}.{last.getFullYear()}
              </Text>
            ) : (
              <Text style={styles.lowOpacityText}>--</Text>
            )}
            {/* enables a reload button if a device is connected */}
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
      </View>

      {/* the modal where the available devices are listed */}
      <DeviceModal
        closeModal={hideModal}
        visible={isModalVisible}
        connectToPeripheral={connectToDevice}
        devices={allDevices} />
    </SafeAreaProvider >
  );
}

// css stylesheet for UI
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
  connectButtonDisabled: {
    opacity: 0.5,
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
  centered: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
});

export default App;