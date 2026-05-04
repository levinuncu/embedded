# PiezoFly - PiezoApp

## How to start
To run the app, move to the `PiezoApp` directory and run the command `npm start`. To see the app on your phone, scan the QR code which will appear in the Terminal.

For Android:
1. Download Expo Go (found in Play Store)
2. Scan QR code through app

For iOS:
1. Download Expo Go (found in App Store)
2. Scan through normal Camera App (which will redirect you to the app)

## Helpful commands
* to install ble library for react native: `npx expo install react-native-ble-plx`
* to install API stuff needed for the connection to the phones: `npx expo install expo-device react-native-base64`
* to install the eas cli: `npx npm install eas-cli`
* to prebuild expo: `npx expo prebuild`
* to clean prebuild expo: `npx expo prebuild --clean`
* to install dev client locally: `npx expo install expo-dev-client`
* to run on android: `npx expo run:android`
* to clear cache: `npm cache clean --force`

## For the prebuild command
Go to the following file path `embedded/frontend/PiezoApp/`
* for android continue the path with: `android/app/src/main/AndroidManifest.xml` and add the following piece of code in the file:
  `<uses-feature android:name="android.permission.BLUETOOTH_CONNECT"/>`
  `<uses-feature android:name="android.hardware.bluetooth_le" android:required="true"/>`
* for ios continue the path with: `ios/PiezoApp/Info.plist` and add the following piece of code into the file:
  `<key>NSBluetoothPeripheralUsageDescription</key>`
	`<string>Allow $(PRODUCT_NAME) to connect to bluetooth devices</string>`

## Troubleshooting
After a few commands, running `npm i` is recommended.
