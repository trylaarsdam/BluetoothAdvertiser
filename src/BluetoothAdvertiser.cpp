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

BleCharacteristic tabletBatteryLevelCharacteristic("tabletBatteryLevel", BleCharacteristicProperty::NOTIFY, BleUuid("c254151d-6b19-438c-a001-5e4d12bc5946"), serviceUuid);

void configureBLE()
{
  BLE.setDeviceName("Tablet");
  BLE.addCharacteristic(tabletBatteryLevelCharacteristic);

  BleAdvertisingData advData;

  // Advertise our private service only
  advData.appendServiceUUID(serviceUuid);
  
  // Continuously advertise when not connected
  BLE.advertise(&advData);
  uint8_t uptime = 0x35;

  tabletBatteryLevelCharacteristic.setValue(uptime);

  // delay(3000);
  // BLE.stopAdvertising();
}

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  configureBLE();
  delay(5000);
}