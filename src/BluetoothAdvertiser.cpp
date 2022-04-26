/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/kevinmcquown/Documents/Projects/Mayo/BluetoothAdvertiser/src/BluetoothAdvertiser.ino"
void configureBLE();
void setup();
void loop();
#line 1 "/Users/kevinmcquown/Documents/Projects/Mayo/BluetoothAdvertiser/src/BluetoothAdvertiser.ino"
SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

/*
 * Project BluetoothAdvertiser
 * Description:
 * Author: trylaarsdam
 * Date:
 */

const BleUuid serviceUuid("b025071e-09df-418a-beff-f64aea621700");

BleCharacteristic insBatteryLevelCharacteristicUuid("insBatteryLevel", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("b026072e-09df-418a-beff-f64aea621766"), serviceUuid);
BleCharacteristic insConnectionCharacteristicUuid("insConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("B027073E-09DF-418A-BEFF-F64AEA621767"), serviceUuid);
BleCharacteristic ctmConnectionCharacteristicUuid("ctmConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("F89528DD-02B2-4479-90BA-F45CB6CD5B73"), serviceUuid);
BleCharacteristic tabletBatteryLevelCharacteristicUuid("tabletBatteryLevel", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("c254151d-6b19-438c-a001-5e4d12bc5946"), serviceUuid);
BleCharacteristic tabletDiskSpaceCharacteristicUuid("tabletDiskSpace", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("c256151d-6b19-438c-a001-5e4d12bc5949"), serviceUuid);
BleCharacteristic tabletConnectionCharacteristicUuid("tabletConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("C255151D-6B19-438C-A001-5E4D12BC5947"), serviceUuid);
BleCharacteristic seizureAlertCharacteristicUuid("seizureAlert", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("8329a311-aedd-4ffe-b0ab-def64a994b53"), serviceUuid);

BleAdvertisingData advData;

void configureBLE()
{
  BLE.setDeviceName("Tablet");
  BLE.addCharacteristic(tabletBatteryLevelCharacteristicUuid);
  BLE.addCharacteristic(insBatteryLevelCharacteristicUuid);
  BLE.addCharacteristic(insConnectionCharacteristicUuid);
  BLE.addCharacteristic(ctmConnectionCharacteristicUuid);
  BLE.addCharacteristic(tabletConnectionCharacteristicUuid);
  BLE.addCharacteristic(tabletDiskSpaceCharacteristicUuid);
  BLE.addCharacteristic(seizureAlertCharacteristicUuid);

  // Advertise our private service only
  advData.appendServiceUUID(serviceUuid);  
}

// void button_handler(system_event_t event, int duration, void* )
// {
//   if (!duration) {
//     digitialWrite(D7, HIGH);
//   }
// }

// setup() runs once, when the device is first turned on.
void setup() {
  pinMode(D7, OUTPUT);
//  System.on(button_status, button_handler)
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  configureBLE();
  insBatteryLevelCharacteristicUuid.setValue(45);
  insConnectionCharacteristicUuid.setValue(1);
  ctmConnectionCharacteristicUuid.setValue(1);
  tabletBatteryLevelCharacteristicUuid.setValue(85);
  tabletDiskSpaceCharacteristicUuid.setValue(55);
  tabletConnectionCharacteristicUuid.setValue(1);

  digitalWrite(D7, HIGH);
  BLE.advertise(&advData);
  delay(5000);
  BLE.stopAdvertising();
  BLE.disconnect();
  digitalWrite(D7, LOW);
  delay(5 * 60 * 1000);
}