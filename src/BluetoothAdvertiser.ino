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

void connectCallback(const BlePeerDevice& peer, void* context){
  // Serial.println("BLE_Connected");
  digitalWrite(D7, HIGH);
}

void disconnectCallback(const BlePeerDevice& peer, void* context){
  // Serial.println("BLE_Disconnected");
  digitalWrite(D7, LOW);
}

void configureBLE()
{
  // BLE.on();
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

void seizureAlert(void);

// setup() runs once, when the device is first turned on.
void setup() {
  BLE.on();
  BLE.onConnected(connectCallback);
  BLE.onDisconnected(disconnectCallback);
  pinMode(D7, OUTPUT);
  pinMode(D6, INPUT_PULLUP);
  attachInterrupt(D6, seizureAlert, FALLING);
  configureBLE();
  Serial.begin(9600);
//  System.on(button_status, button_handler)
}

uint64_t timer = 0;
bool seizure = false;
float insBattery = 100;
float tabletBattery = 100;
float tabletDiskSpace = 100;

void loop() {
  if(timer % (4 * 60) == 0) {
    Serial.printlnf("Triggered at %d minutes", timer / (4 * 60));
    insBatteryLevelCharacteristicUuid.setValue((int)insBattery);
    insConnectionCharacteristicUuid.setValue(1);
    ctmConnectionCharacteristicUuid.setValue(1);
    tabletBatteryLevelCharacteristicUuid.setValue((int)tabletBattery);
    tabletDiskSpaceCharacteristicUuid.setValue((int)tabletDiskSpace);
    tabletConnectionCharacteristicUuid.setValue(1);

    insBattery = insBattery - 0.5;
    tabletBattery = tabletBattery - 0.7;
    tabletDiskSpace = tabletDiskSpace - 0.3;

    if(insBattery < 0) {
      insBattery = 0;
    }
    if(tabletBattery < 0) {
      tabletBattery = 0;
    }
    if(tabletDiskSpace < 0) {
      tabletDiskSpace = 0;
    }
    BLE.on();

    BLE.advertise(&advData);
    delay(5000);
    BLE.stopAdvertising();
    BLE.off();
  }

  if(seizure == true) {
    if(digitalRead(D6) == LOW) {
      seizureAlertCharacteristicUuid.setValue(1);
      BLE.advertise(&advData);
      delay(5000);
      BLE.stopAdvertising();
      BLE.off();
      seizureAlertCharacteristicUuid.setValue(0);
      seizure = false;
      // delay(5000);
      // BLE.stopAdvertising();
    }
  }
  
  timer++;
  delay(1000);
}

void seizureAlert()
{
  seizure = true;
}