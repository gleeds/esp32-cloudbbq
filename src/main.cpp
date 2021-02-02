#include <Arduino.h>
#include <BLEDevice.h>

BLEScan* pBLEScan;
BLEClient* piBBQClient;
boolean paired = false;
static BLEAddress *piBBQDeviceAddress;
static BLERemoteService* piBBQPrimaryService;
static BLERemoteCharacteristic* piBBQTempCharacteristic;
static BLERemoteCharacteristic* piBBQPairCharacteristic;
static BLERemoteCharacteristic* piBBQCmdCharacteristic;

static BLEUUID iBBQServiceUUID("0000fff0-0000-1000-8000-00805f9b34fb");
static BLEUUID iBBQPairCharacteristicUUID("0000fff2-0000-1000-8000-00805f9b34fb");
static BLEUUID iBBQTempCharacteristicUUID("0000fff4-0000-1000-8000-00805f9b34fb");
static BLEUUID iBBQCmdCharacteristicUUID("0000fff5-0000-1000-8000-00805f9b34fb");
static std::string iBBQName = "iBBQ";
static uint8_t pairKeyByte[] = {
  0x22,
  0x07,
  0x06,
  0x05,
  0x04,
  0x03,
  0x02,
  0x01,
  0xB8,
  0x22,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00
};
static std::string pairKey(reinterpret_cast<char*>(pairKeyByte),sizeof(pairKeyByte));

static uint8_t startCmdByte[] = {0x11,0x01,0x00,0x00,0x00,0x00};
static std::string iBBQStartCmd(reinterpret_cast<char*>(startCmdByte),sizeof(startCmdByte));


static void tempNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify)
{
  Serial.println("Data Notification Received");
  // std::string stringData((char*)data,length);
  // Serial.println(stringData.c_str());
}

static void connectToDevice(BLEAddress deviceAddress)
{
  piBBQClient = BLEDevice::createClient();
  piBBQClient->connect(deviceAddress);
  Serial.println("Connected to iBBQ");
  paired=true;
  piBBQPrimaryService = piBBQClient->getService(iBBQServiceUUID);
  if (piBBQPrimaryService != nullptr)
  {
    Serial.println("Found Primary Service");
    piBBQPairCharacteristic = piBBQPrimaryService->getCharacteristic(iBBQPairCharacteristicUUID);
    if (piBBQPairCharacteristic != nullptr)
    {
      Serial.println("Found Pair Characteristic");
      // piBBQPairCharacteristic->writeValue("2107060504030201b8220000000000");
      piBBQPairCharacteristic->writeValue(pairKeyByte,sizeof(pairKeyByte));
      // piBBQPairCharacteristic->writeValue(pairKey);
      Serial.println("Wrote Pair Key");
      Serial.println(pairKey.c_str());
      piBBQTempCharacteristic = piBBQPrimaryService->getCharacteristic(iBBQTempCharacteristicUUID);
      piBBQCmdCharacteristic = piBBQPrimaryService->getCharacteristic(iBBQCmdCharacteristicUUID);
      if (piBBQTempCharacteristic != nullptr && piBBQCmdCharacteristic != nullptr)
      {
        Serial.println("Found Temp Characteristic");
        piBBQTempCharacteristic->registerForNotify(tempNotifyCallback);
        Serial.println("Registered notification callback");
        piBBQCmdCharacteristic->writeValue(startCmdByte,sizeof(startCmdByte));
        Serial.println("Sent Start Command");
      }
    }
  }


}


class DeviceFoundCallback: public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice){
    // Serial.printf("Found Device: %s \n", advertisedDevice.toString().c_str());
    if (advertisedDevice.haveName() && advertisedDevice.getName().c_str() == iBBQName)
    {
      Serial.println("Found iBBQ by Name");
      piBBQDeviceAddress =  new BLEAddress(advertisedDevice.getAddress());
    }
    else if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(iBBQServiceUUID))
    {
      // iBBQ appers to only advertise it's LocalName in response to an active scan, so if the callback doesn't
      // catch the broadcast on the first time, we need to fall back to ServiceUUID.  I think this is a common 
      // value accross diffrent devices in the family but I'm not sure.
      Serial.println("Found iBBQ by ServiceUUID");
      piBBQDeviceAddress =  new BLEAddress(advertisedDevice.getAddress());
    }
  }
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new DeviceFoundCallback());
  pBLEScan->setActiveScan(true);
}

void loop() {
  pBLEScan->start(3);
  if (!paired && piBBQDeviceAddress != nullptr)
  {
    connectToDevice(*piBBQDeviceAddress);
    Serial.println("Done with connecting...");
  }
  while(paired)
  {
    
  }
}