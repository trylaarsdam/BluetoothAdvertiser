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

// Amount of time we stay connected to give phone ample time to read values from all characteristics
// This can be a somewhat lengthy time when app is in background
#define kConnectionTime 5000

#define kWaitingTimerBetweenAdvertisements 30000

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t packetNumber;

String statusServiceUUID = String("b025071e-09df-418a-beff-f64aea62170");
String serviceUUIDString = statusServiceUUID + String(getPacketNumber());

String seizureServiceUUID = String("31410683-B0E4-434B-A745-4F1DB7AB570");
String seizureUUIDString = seizureServiceUUID + String(getPacketNumber());

const BleUuid serviceUuid(serviceUUIDString.c_str());
const BleUuid seizureUuid(seizureUUIDString.c_str());

BleCharacteristic insBatteryLevelCharacteristicUuid("insBatteryLevel", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("b026072e-09df-418a-beff-f64aea621766"), serviceUuid);
BleCharacteristic insConnectionCharacteristicUuid("insConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("B027073E-09DF-418A-BEFF-F64AEA621767"), serviceUuid);
BleCharacteristic ctmConnectionCharacteristicUuid("ctmConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("F89528DD-02B2-4479-90BA-F45CB6CD5B73"), serviceUuid);
BleCharacteristic tabletBatteryLevelCharacteristicUuid("tabletBatteryLevel", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("c254151d-6b19-438c-a001-5e4d12bc5946"), serviceUuid);
BleCharacteristic tabletDiskSpaceCharacteristicUuid("tabletDiskSpace", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("c256151d-6b19-438c-a001-5e4d12bc5949"), serviceUuid);
BleCharacteristic tabletConnectionCharacteristicUuid("tabletConnection", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("C255151D-6B19-438C-A001-5E4D12BC5947"), serviceUuid);
BleCharacteristic seizureAlertCharacteristicUuid("seizureAlert", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("8329a311-aedd-4ffe-b0ab-def64a994b53"), serviceUuid);

BleAdvertisingData advData;
BleAdvertisingData seizureAdvData;

enum states
{
  waiting,
  startAdvertising,
  advertising,
  connected,
  disconnect,
  cancelAdvertising,
  setSeizureDetect
};
enum wifiStates
{
  waitingToConnect,
  connectingToParticle,
  connectedToParticle
};

states state;
wifiStates wifiState;

bool seizureFastModeOn = false;
bool seizureDetectTrigger = false;

unsigned long currentElapstedTimeToConnect = 0;
unsigned long finalElapsedTimeToConnect = 0;
unsigned long totalFinalElapsedTimeToConnect = 0;
unsigned long minTimeToConnect = 500000;
unsigned long maxTimeToConnect = 0;
unsigned long connectionCount = 0;

unsigned long waitingTimer = 0;
unsigned long advertisingTimer = 0;
unsigned long connectionTimer = 0;

bool waitingTimerTriggered = false;
bool advertisingTimerTriggered = false;
bool connectionTimerTriggered = false;

bool updatingAdvertisingAddress = false;
bool onWiFi = false;
unsigned long waitForWiFiTimer = 5000 + millis();

uint8_t buf[BLE_MAX_ADV_DATA_LEN];

void connectCallback(const BlePeerDevice &peer, void *context)
{
  state = connected;
  finalElapsedTimeToConnect = millis() - currentElapstedTimeToConnect;
  connectionCount++;
  minTimeToConnect = min(minTimeToConnect, finalElapsedTimeToConnect);
  maxTimeToConnect = max(maxTimeToConnect, finalElapsedTimeToConnect);
  totalFinalElapsedTimeToConnect = totalFinalElapsedTimeToConnect + finalElapsedTimeToConnect;
  BLE.stopAdvertising();
  advertisingTimer = 0;
  waitingTimer = 0;
  connectionTimer = millis() + kConnectionTime;
}

void disconnectCallback(const BlePeerDevice &peer, void *context)
{
  state = waiting;
  connectionTimer = 0;
  advertisingTimer = 0;
  waitingTimer = millis() + (kWaitingTimerBetweenAdvertisements);
}

// void setAdvertisingManufacturingData(bool alert)
// {
//   if (alert)
//   {
//     buf[0] = 0xFF;
//     buf[1] = 0xFF;
//     buf[2] = seizureFastModeOn ? 0x01 : 0x00;
//   }
//   else
//   {
//     buf[0] = 0xFF;
//     buf[1] = 0xFF;
//     buf[2] = 0x00;
//   }
//   advData.appendCustomData(buf, 3);
// }

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
  seizureAdvData.appendServiceUUID(seizureUuid);
}

// setup() runs once, when the device is first turned on.
void setup()
{
  WiFi.setCredentials("ncc1701d","iggyogden1");
  WiFi.setCredentials("WCL","atmega328");
  WiFi.setCredentials("LoopTracks", "atmega328");
  Serial.begin(9600);
  packetNumber = EEPROM.read(0x00);
  if (packetNumber == 0xFF) {
    packetNumber = 0;
    EEPROM.put(0x00, packetNumber);
  }
  //  SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("ePad Sim v1.2");
  display.println("<- A Join WiFi");
  display.println("<- B Adv Packet");
  display.display();
  while ((millis() < waitForWiFiTimer) && !(onWiFi || updatingAdvertisingAddress))
  {
    if (!digitalRead(BUTTON_A))
    {
      onWiFi = true;
      WiFi.on();
      WiFi.connect();
      wifiState = waitingToConnect;
    }
    if (!digitalRead(BUTTON_B))
    {
      updatingAdvertisingAddress = true;
    }
  }

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

void loop()
{
  if (updatingAdvertisingAddress)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setCursor(0, 0);
    display.println("A < +");
    display.println("");
    display.println("");
    display.println("C < -");
    display.setCursor(75, 13);
    display.setTextSize(2);
    display.print(packetNumber, HEX);
    display.setTextSize(1);
    display.display();
    if (!digitalRead(BUTTON_A))
    {
      packetNumber++;
      packetNumber = packetNumber % 10;
      EEPROM.put(0x00, packetNumber);
      delay(100);
    }
    if (!digitalRead(BUTTON_C))
    {
      if (packetNumber > 0) {
        packetNumber--;
        EEPROM.put(0x00, packetNumber);
      }
      delay(100);
    }
    return;
  }
  // This mode is used to get on WiFi for OTA updates.
  if (onWiFi)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    switch (wifiState)
    {
    case waitingToConnect:
      display.println("Joining WiFi...");
      display.println("");
      display.println("Antenna Connected?");
      if (WiFi.ready())
      {
        wifiState = connectingToParticle;
        Particle.connect();
      }
      break;
    case connectingToParticle:
      display.println("Finding Particle ...");
      if (Particle.connected())
      {
        wifiState = connectedToParticle;
      }
      break;
    case connectedToParticle:
      display.println("Waiting for OTA ...");
      break;
    }
    display.display();
    return;
  }

  // Start of main loop functionality
  handleTimedEvents();
  if (!digitalRead(BUTTON_A))
  {
    if (state == waiting)
    {
      state = startAdvertising;
      currentElapstedTimeToConnect = millis();
    }
  }

  if (!digitalRead(BUTTON_B))
  {
    seizureFastModeOn = !seizureFastModeOn;
    delay(200);
  }

  if (!digitalRead(BUTTON_C))
  {
    if (state == waiting)
    {
      setSeizure(); // updates characteristic value with a 1
      seizureDetectTrigger = true;
      state = startAdvertising;
      currentElapstedTimeToConnect = millis();
    }
  }
  switch (state)
  {
  case waiting:
    clearSeizure();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("A <- Advertise    ");
    display.print((waitingTimer - millis()) / 1000);
    display.println("s");
    if (seizureFastModeOn)
    {
      display.println("B <- Fast Mode: On");
    }
    else
    {
      display.println("B <- Fast Mode: Off");
    }
    display.println("C <- Seizure Detect!");
    if (connectionCount > 0)
    {
      // display.drawLine(0,24,128,24,1);
      display.setCursor(0, 25);
      display.print(minTimeToConnect);
      display.print(" - ");
      display.print(totalFinalElapsedTimeToConnect / connectionCount);
      display.print(" - ");
      display.print(maxTimeToConnect);
      display.print(" ");
      display.println(connectionCount);
    }
    display.display();
    if (waitingTimerTriggered)
    {
      state = startAdvertising;
      currentElapstedTimeToConnect = millis();
    }
    break;
  case startAdvertising:
    connectionTimer = 0;
    advertisingTimer = millis() + 10000; // advertise for a max of 10 seconds then give up
    waitingTimer = 0;
    updateCharacteristicValues();
    if (seizureDetectTrigger && seizureFastModeOn) {
      BLE.advertise(&seizureAdvData);
    } else {
      BLE.advertise(&advData);
    }
    state = advertising;
    break;
  case advertising:
    digitalWrite(D7, HIGH);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    if (seizureDetectTrigger && seizureFastModeOn) {
      display.print("   ...adv: seizure...");
    } else {
      display.print("   ...adv: normal...");
    }
    display.setCursor(30, 18);
    display.setTextSize(2);
    display.print((millis() - currentElapstedTimeToConnect));
    display.print(" ms");
    display.display();
    display.setTextSize(1);
    if (advertisingTimerTriggered)
    {
      state = cancelAdvertising;
    }
    break;
  case cancelAdvertising:
    digitalWrite(D7, LOW);
    BLE.stopAdvertising();
    seizureDetectTrigger = false;
    connectionTimer = 0;
    advertisingTimer = 0;
    waitingTimer = millis() + (kWaitingTimerBetweenAdvertisements);
    state = waiting;
    break;
  case connected:
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("connected in ...");
    display.setCursor(30, 16);
    display.setTextSize(2);
    display.print(finalElapsedTimeToConnect);
    display.print(" ms");
    // display.setTextSize(1);
    // display.setCursor(100,0);
    // display.print((connectionTimer - millis())/1000);
    display.display();
    display.setTextSize(1);
    if (connectionTimerTriggered)
    {
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
    state = waiting;
  }
}

void setSeizure()
{
  seizureAlertCharacteristicUuid.setValue(1);
}
void clearSeizure()
{
  seizureAlertCharacteristicUuid.setValue(0);
}

void updateCharacteristicValues()
{
  insBatteryLevelCharacteristicUuid.setValue((int)insBattery);
  insConnectionCharacteristicUuid.setValue(1);
  ctmConnectionCharacteristicUuid.setValue(1);
  tabletBatteryLevelCharacteristicUuid.setValue((int)tabletBattery);
  tabletDiskSpaceCharacteristicUuid.setValue((int)tabletDiskSpace);
  tabletConnectionCharacteristicUuid.setValue(1);

  insBattery = insBattery - 0.2;
  tabletBattery = tabletBattery - 0.1;
  tabletDiskSpace = tabletDiskSpace - 0.05;

  if (insBattery < 0)
  {
    insBattery = 0;
    resetBatteryLevels();
  }
  if (tabletBattery < 0)
  {
    tabletBattery = 0;
    resetBatteryLevels();
  }
  if (tabletDiskSpace < 0)
  {
    tabletDiskSpace = 0;
    resetBatteryLevels();
  }
}
void resetBatteryLevels()
{
  insBattery = 100;
  tabletBattery = 100;
  tabletDiskSpace = 100;
}

void handleTimedEvents()
{
  connectionTimerTriggered = false;
  waitingTimerTriggered = false;
  advertisingTimerTriggered = false;
  if (connectionTimer > 0)
  {
    if (connectionTimer < millis())
    {
      connectionTimerTriggered = true;
      connectionTimer = 0;
    }
  }
  if (advertisingTimer > 0)
  {
    if (advertisingTimer < millis())
    {
      advertisingTimerTriggered = true;
      advertisingTimer = 0;
    }
  }
  if (waitingTimer > 0)
  {
    if (waitingTimer < millis())
    {
      waitingTimerTriggered = true;
      waitingTimer = 0;
    }
  }
}
uint8_t getPacketNumber() {
  uint8_t number = EEPROM.read(0x00);
  if (number == 0xFF) {
    number = 0;
    EEPROM.write(0x00, 0);
  }
  return number;
}