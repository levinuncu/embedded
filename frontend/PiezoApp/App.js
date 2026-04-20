import React, { useState } from 'react';
import { StyleSheet, Button, View, Text, Alert, Platform } from 'react-native';
import { SafeAreaView, SafeAreaProvider } from 'react-native-safe-area-context';

function showAlert(message) {
  Alert.alert('Connection', message)
}

const App = () => {
  const [isConnected, setIsConnected] = useState(false);
  return (
    <SafeAreaProvider>
      <SafeAreaView style={styles.container}>
        <View>
          <Text style={styles.title}>App connection working!</Text>
          <Text style={styles.title}>current (mock) connection state: {isConnected ? 'connected' : 'disconnected'}</Text>
          <View style={styles.fixToText}>
            <Button color="purple" title="Connect" onPress={() => { setIsConnected(true); showAlert('Connected'); }} disabled={isConnected} />
            <Button color="purple" title="Disconnect" onPress={() => { setIsConnected(false); showAlert('Disconnected'); }} disabled={!isConnected} />
          </View>
        </View>
      </SafeAreaView>
    </SafeAreaProvider>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#fff',
    alignItems: 'center',
    justifyContent: 'center',
    marginHorizontal: 16,
  },
  title: {
    textAlign: 'center',
    marginVertical: 8,
  },
  fixToText: {
    flexDirection: 'row',
    justifyContent: 'space-between',
  },
});

export default App;