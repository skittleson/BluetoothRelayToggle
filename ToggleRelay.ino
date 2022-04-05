#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "2396a27a-45cd-11ec-81d3-0242ac130003"
#define CHARACTERISTIC_UUID "2e08641e-45cd-11ec-81d3-0242ac130003"
#define CHIP_CHARACTERISTIC_UUID "dd37f38f-ff65-45b1-a7b9-ca8f017a8ec0"
uint32_t chipId = 0;
int gateTrigger = 0;
bool deviceConnected = false;
bool oldDeviceConnected = false;
BLEServer *pServer = NULL;

class RelayCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *relayCharacteristic) {

    // Get value from the callback service then if it true, set the gate trigger
    // to true. otherwise set to default again. toggle logic.
    std::string value = relayCharacteristic->getValue();
    Serial.println(value.c_str());
    if (value == "1") {
      gateTrigger = 1;
    } else {
      relayCharacteristic->setValue(gateTrigger);
    }
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    BLEDevice::startAdvertising();
  };

  void onDisconnect(BLEServer *pServer) { deviceConnected = false; }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  BLEDevice::init("ToggleRelay");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *relayCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  BLECharacteristic *chirelayCharacteristic = pService->createCharacteristic(
      CHIP_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  relayCharacteristic->setCallbacks(new RelayCallbacks());
  relayCharacteristic->setValue(gateTrigger);

  // set chip id
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  chirelayCharacteristic->setValue(chipId);

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(
      0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  digitalWrite(LED_BUILTIN, LOW);
  if (gateTrigger == 1) {
    Serial.println("trigger");
    digitalWrite(LED_BUILTIN, HIGH);
    gateTrigger = 0;
    delay(2000);
  }
}