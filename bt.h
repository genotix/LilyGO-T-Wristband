// Bluetooth functions

#ifdef ENABLE_BLE_DATA_TRANSMISSION
#ifndef ENABLE_SENSOR
#define ENABLE_SENSOR
#endif
#endif

#ifdef  ENABLE_BLE_DATA_TRANSMISSION
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SENSOR_SERVICE_UUID                 "4fafc301-1fb5-459e-8fcc-c5c9c331914b"
#define SENSOR_CHARACTERISTIC_UUID          "beb5483c-36e1-4688-b7f5-ea07361b26a8"
static BLECharacteristic *pSensorCharacteristic = nullptr;
static BLEDescriptor  *pSensorDescriptor = nullptr;

static bool deviceConnected = false;
static bool enableNotify = false;

class DeviceServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;

        #ifdef ENABLE_SERIAL            
          Serial.println("Device connect");
        #endif  
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        enableNotify = false;

        #ifdef ENABLE_SERIAL            
          Serial.println("Device disconnect");
        #endif  
    }
};

class SensorDescriptorCallbacks: public BLEDescriptorCallbacks
{
    void onWrite(BLEDescriptor *pDescriptor)
    {
        uint8_t *value = pDescriptor->getValue();
        #ifdef ENABLE_SERIAL            
          Serial.print("SensorDescriptorCallbacks:");
        #endif  
        enableNotify = value[0] ? true : false;
    }
    void onRead(BLEDescriptor *pDescriptor)
    {

    }
};
#endif


void setupBLE()
{
#ifdef  ENABLE_BLE_DATA_TRANSMISSION
    BLEDevice::init("T-Wristband");


    BLEServer *pServer = BLEDevice::createServer();

    pServer->setCallbacks(new DeviceServerCallbacks());

    /*Sensor Service*/
    BLEService *pSensorService = pServer->createService(SENSOR_SERVICE_UUID);

    pSensorCharacteristic = pSensorService->createCharacteristic(
                                SENSOR_CHARACTERISTIC_UUID,
                                BLECharacteristic::PROPERTY_READ   |
                                BLECharacteristic::PROPERTY_NOTIFY
                            );
    pSensorCharacteristic->addDescriptor(new BLE2902());

    pSensorDescriptor = pSensorCharacteristic->getDescriptorByUUID("2902");

    pSensorDescriptor->setCallbacks(new SensorDescriptorCallbacks());

    pSensorService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

    pAdvertising->addServiceUUID(SENSOR_SERVICE_UUID);

    pAdvertising->setScanResponse(true);

    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue

    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();

#endif
}

void BLE_Transmission()
{
#ifdef ENABLE_BLE_DATA_TRANSMISSION
    typedef struct {
        float ax;
        float ay;
        float az;
        float gx;
        float gy;
        float gz;
        float mx;
        float my;
        float mz;
    } imu_data_t;

    static uint32_t updataRate = 0;

    if (millis() - updataRate < 1000) {
        return;
    }
    getIMU();

    updataRate = millis();

    if (deviceConnected && enableNotify) {
        // Create a structure to send data
        imu_data_t data;
        data.ax = imu.calcAccel(imu.ax);
        data.ay = imu.calcAccel(imu.ay);
        data.az = imu.calcAccel(imu.az);
        data.gx = imu.calcGyro(imu.gx);
        data.gy = imu.calcGyro(imu.gy);
        data.gz = imu.calcGyro(imu.gz);
        data.mx = imu.calcMag(imu.mx);
        data.my = imu.calcMag(imu.my);
        data.mx = imu.calcMag(imu.mx);

        pSensorCharacteristic->setValue((uint8_t *)&data, sizeof(imu_data_t));

        pSensorCharacteristic->notify();

        uint8_t *ptr = (uint8_t *)&data;
        #ifdef ENABLE_SERIAL            
          Serial.print("[");
          Serial.print(millis());
          Serial.print("]");

          for (int i = 0; i < sizeof(data); ++i) {
              Serial.print(ptr[i], HEX);
              Serial.print(" ");
          }
          Serial.println();
        #endif  
    }
#endif
}


void sleepBT() {
    btStop();

    sleepStateBT     = true;
}
