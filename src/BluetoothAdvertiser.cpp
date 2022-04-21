/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/trylaarsdam/Documents/dev/BluetoothAdvertiser/src/BluetoothAdvertiser.ino"
void configureBLE();
void setup();
void loop();
#line 1 "/Users/trylaarsdam/Documents/dev/BluetoothAdvertiser/src/BluetoothAdvertiser.ino"
SYSTEM_MODE(MANUAL);
/*
 * Project BluetoothAdvertiser
 * Description:
 * Author: trylaarsdam
 * Date:
 */

const BleUuid serviceUuid("b025071e-09df-418a-beff-f64aea621700");

BleCharacteristic insBatteryLevelCharacteristicUuid("insBatteryLevel", BleCharacteristicProperty::NOTIFY, BleUuid("b026072e-09df-418a-beff-f64aea621766"), serviceUuid);
BleCharacteristic insConnectionCharacteristicUuid("insConnection", BleCharacteristicProperty::NOTIFY, BleUuid("B027073E-09DF-418A-BEFF-F64AEA621767"), serviceUuid);
BleCharacteristic ctmBatteryLevelCharacteristicUuid("ctmBatterLevel", BleCharacteristicProperty::NOTIFY, BleUuid("f89428dd-02b2-4479-90ba-f45cb6cd5b72"), serviceUuid);
BleCharacteristic ctmConnectionCharacteristicUuid("ctmConnection", BleCharacteristicProperty::NOTIFY, BleUuid("F89528DD-02B2-4479-90BA-F45CB6CD5B73"), serviceUuid);
BleCharacteristic tabletBatteryLevelCharacteristicUuid("tabletBatteryLevel", BleCharacteristicProperty::NOTIFY, BleUuid("c254151d-6b19-438c-a001-5e4d12bc5946"), serviceUuid);
BleCharacteristic tabletDiskSpaceCharacteristicUuid("tabletDiskSpace", BleCharacteristicProperty::NOTIFY, BleUuid("c256151d-6b19-438c-a001-5e4d12bc5949"), serviceUuid);
BleCharacteristic tabletConnectionCharacteristicUuid("tabletConnection", BleCharacteristicProperty::NOTIFY, BleUuid("C255151D-6B19-438C-A001-5E4D12BC5947"), serviceUuid);
BleCharacteristic seizureAlertCharacteristicUuid("seizureAlert", BleCharacteristicProperty::NOTIFY, BleUuid("8329a311-aedd-4ffe-b0ab-def64a994b53"), serviceUuid);

void configureBLE()
{
  BLE.setDeviceName("Tablet");
  BLE.addCharacteristic(tabletBatteryLevelCharacteristicUuid);
  BLE.addCharacteristic(insBatteryLevelCharacteristicUuid);
  BLE.addCharacteristic(insConnectionCharacteristicUuid);
  BLE.addCharacteristic(ctmBatteryLevelCharacteristicUuid);
  BLE.addCharacteristic(ctmConnectionCharacteristicUuid);
  BLE.addCharacteristic(tabletConnectionCharacteristicUuid);
  BLE.addCharacteristic(tabletDiskSpaceCharacteristicUuid);
  BLE.addCharacteristic(seizureAlertCharacteristicUuid);

  BleAdvertisingData advData;

  // Advertise our private service only
  advData.appendServiceUUID(serviceUuid);
  
  // Continuously advertise when not connected
  BLE.advertise(&advData);
}

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  configureBLE();
  insBatteryLevelCharacteristicUuid.setValue(0x01);
  insConnectionCharacteristicUuid.setValue(0x02);
  ctmBatteryLevelCharacteristicUuid.setValue(0x03);
  ctmConnectionCharacteristicUuid.setValue(0x04);
  tabletBatteryLevelCharacteristicUuid.setValue(0x05);
  tabletDiskSpaceCharacteristicUuid.setValue(0x06);
  tabletConnectionCharacteristicUuid.setValue(0x07);
  delay(5000);
}