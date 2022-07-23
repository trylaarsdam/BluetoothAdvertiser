SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define BUTTON_A D4
#define BUTTON_B D3
#define BUTTON_C D2
#define SSD1306_WHITE 1
//#define kWaitingTimerBetweenAdvertisements 4 * 60000
#define kWaitingTimerBetweenAdvertisements 30000

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const BleUuid serviceUuid("b025071e-09df-418a-beff-f64aea621700");

BleCharacteristic insBatteryLevelCharacteristicUuid("insBatteryLevel", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("b026072e-09df-418a-beff-f64aea621766"), serviceUuid);
BleCharacteristic insConnectionCharacteristicUuid("insConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("B027073E-09DF-418A-BEFF-F64AEA621767"), serviceUuid);
BleCharacteristic ctmConnectionCharacteristicUuid("ctmConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("F89528DD-02B2-4479-90BA-F45CB6CD5B73"), serviceUuid);
BleCharacteristic tabletBatteryLevelCharacteristicUuid("tabletBatteryLevel", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("c254151d-6b19-438c-a001-5e4d12bc5946"), serviceUuid);
BleCharacteristic tabletDiskSpaceCharacteristicUuid("tabletDiskSpace", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("c256151d-6b19-438c-a001-5e4d12bc5949"), serviceUuid);
BleCharacteristic tabletConnectionCharacteristicUuid("tabletConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("C255151D-6B19-438C-A001-5E4D12BC5947"), serviceUuid);
BleCharacteristic seizureAlertCharacteristicUuid("seizureAlert", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("8329a311-aedd-4ffe-b0ab-def64a994b53"), serviceUuid);

BleAdvertisingData advData;

enum states{waiting, startAdvertising, advertising, connected, disconnect, cancelAdvertising, setSeizureDetect};

states state;

// void connectionTimerExpired() {
//     Serial.println("Connection timer fired...");
//     state = disconnect;
// }
// Timer connectedTimer(5000, connectionTimerExpired, true);

// void advertisingTimerFired() {
//     Serial.println("Advertising timer fired...");
//     state = cancelAdvertising;
// }
// Timer advertisingTimer(10000, advertisingTimerFired, true);

// void waitingTimerFired() {
//     Serial.println("Waiting timer fired...");
//     state = startAdvertising;
// }
// Timer waitingTimer(kWaitingTimerBetweenAdvertisements, waitingTimerFired, true);

unsigned long waitingTimer = 0;
unsigned long advertisingTimer = 0;
unsigned long connectionTimer = 0;
bool waitingTimerTriggered = false;
bool advertisingTimerTriggered = false;
bool connectionTimerTriggered = false;

void connectCallback(const BlePeerDevice& peer, void* context){
  state = connected;
  BLE.stopAdvertising();
  advertisingTimer = 0;
  waitingTimer = 0;
  connectionTimer = millis() + 5000;
  // advertisingTimer.stop();
  // waitingTimer.stop();
  // connectedTimer.start();
}

void disconnectCallback(const BlePeerDevice& peer, void* context){
  state = waiting;
  connectionTimer = 0;
  advertisingTimer = 0;
  waitingTimer = millis() + (kWaitingTimerBetweenAdvertisements);
  // connectedTimer.stop();
  // advertisingTimer.stop();
  // waitingTimer.start();
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

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
  //while (!Serial.isConnected()) {}
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.print("ETablet Simulator");
  display.display();
  delay(1000);

  BLE.on();
  BLE.onConnected(connectCallback);
  BLE.onDisconnected(disconnectCallback);
  pinMode(D7, OUTPUT);
  configureBLE();

  state = waiting;
  connectionTimer = 0;
  advertisingTimer = 0;
  waitingTimer = millis() + (kWaitingTimerBetweenAdvertisements);
}

bool seizure = false;
float insBattery = 100;
float tabletBattery = 100;
float tabletDiskSpace = 100;

void loop() {
  handleTimedEvents();
  if (!digitalRead(BUTTON_A)) {
    if (state == waiting) {
      state = startAdvertising;
    }
  }
  if (!digitalRead(BUTTON_C)) {
    if (state == waiting) {
      setSeizure();
      state = startAdvertising;
    }
  }
  switch(state) {
    case waiting:
        clearSeizure();
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("waiting...");
        display.display();
        if (waitingTimerTriggered) {
          state = startAdvertising;
        }
        break;
    case startAdvertising:
        connectionTimer = 0;
        advertisingTimer = millis() + 10000;
        waitingTimer = 0;
        // waitingTimer.stop();
        // connectedTimer.stop();
        // advertisingTimer.start();
        updateCharacteristicValues();
        BLE.advertise(&advData);
        state = advertising;
        break;
    case advertising:
        digitalWrite(D7, HIGH);
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("advertising...");
        display.display();
        if (advertisingTimerTriggered) {
          state = cancelAdvertising;
        }
        break;
    case cancelAdvertising:
        digitalWrite(D7, LOW);
        BLE.stopAdvertising();
        connectionTimer = 0;
        advertisingTimer = 0;
        waitingTimer = millis() + (kWaitingTimerBetweenAdvertisements);
        // advertisingTimer.stop();
        // waitingTimer.start();
        state = waiting;
        break;
    case connected:
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("connected...");
        display.display();
        if (connectionTimerTriggered) {
          state = disconnect;
        }
        break;
    case disconnect:
        digitalWrite(D7, LOW);
        BLE.disconnectAll();
        BLE.stopAdvertising();
        connectionTimer = 0;
        advertisingTimer = 0;
        waitingTimer = millis() + (kWaitingTimerBetweenAdvertisements);
        // connectedTimer.stop();
        // advertisingTimer.stop();
        // waitingTimer.start();
        state = waiting;
  }
}

void setSeizure() {
    seizureAlertCharacteristicUuid.setValue(1);
}
void clearSeizure() {
    seizureAlertCharacteristicUuid.setValue(0);
}

void updateCharacteristicValues() {
    insBatteryLevelCharacteristicUuid.setValue((int)insBattery);
    insConnectionCharacteristicUuid.setValue(1);
    ctmConnectionCharacteristicUuid.setValue(1);
    tabletBatteryLevelCharacteristicUuid.setValue((int)tabletBattery);
    tabletDiskSpaceCharacteristicUuid.setValue((int)tabletDiskSpace);
    tabletConnectionCharacteristicUuid.setValue(1);

    insBattery = insBattery - 0.2;
    tabletBattery = tabletBattery - 0.1;
    tabletDiskSpace = tabletDiskSpace - 0.05;

    if(insBattery < 0) {
      insBattery = 0;
    }
    if(tabletBattery < 0) {
      tabletBattery = 0;
    }
    if(tabletDiskSpace < 0) {
      tabletDiskSpace = 0;
    }
}

void handleTimedEvents() {
  connectionTimerTriggered = false;
  waitingTimerTriggered = false;
  advertisingTimerTriggered = false;
  if (connectionTimer > 0) {
    if (connectionTimer < millis()) {
      connectionTimerTriggered = true;
      connectionTimer = 0;
    }
  }
  if (advertisingTimer > 0) {
    if (advertisingTimer < millis()) {
      advertisingTimerTriggered = true;
      advertisingTimer = 0;
    }
  }
  if (waitingTimer > 0) {
    if (waitingTimer < millis()) {
      waitingTimerTriggered = true;
      waitingTimer = 0;
    }
  }
}