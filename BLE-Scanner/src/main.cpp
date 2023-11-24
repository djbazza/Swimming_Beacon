#include <M5StickC.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#define GPIO_DEEP_SLEEP_DURATION     250  // sleep x milli seconds and then wake up

unsigned long prev;
//struct timeval now;
unsigned int numVals;
double trigger;

class IBeaconAdvertised: public BLEAdvertisedDeviceCallbacks {
  public:
    double dist;
    unsigned long elapse;

    // BLE検出時のコールバック
    void onResult(BLEAdvertisedDevice device) {
      if (!isIBeacon(device)) {
        return;
      }
      //gettimeofday(&now, NULL);
      /*M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("\nTime: ");
      M5.Lcd.print(millis()-prev);
      M5.Lcd.print("ms \n");*/
      if(!digitalRead(37))
        trigger = dist;
      if((millis() - prev) < GPIO_DEEP_SLEEP_DURATION){
        //if(dist-device.getRSSI()<=2 && dist-device.getRSSI()>=-2)
        dist = (dist*numVals++ + device.getRSSI())/numVals;
        return;
      }
      dist = (dist*numVals++ + device.getRSSI())/numVals;
      if(dist > trigger)
      {
        digitalWrite(10, LOW);
        digitalWrite(32, HIGH);
      }
      else
      {
        digitalWrite(10, HIGH);
        digitalWrite(32, LOW);
      }
      printIBeacon(device);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(2, 2);
      M5.Lcd.print("Samples: ");
      M5.Lcd.print(numVals);
      numVals = 0;
      M5.Lcd.print("\nRSSI: ");
      M5.Lcd.print(device.getRSSI());
      M5.Lcd.print("\nTrigger: ");
      M5.Lcd.print(trigger);
      M5.Lcd.print("\nChange: ");
      M5.Lcd.print(dist - device.getRSSI());
      M5.Lcd.print("\nSpeed: ");
      M5.Lcd.print((dist - device.getRSSI())/((millis() - prev)/1000));
      M5.Lcd.print("\nDistance: ");
      M5.Lcd.print(dist);
      dist = device.getRSSI();
      M5.Lcd.print("\nTime: ");
      M5.Lcd.print(millis() - prev);
      prev = millis();
      M5.Lcd.print("ms \n");      
    }

  private:
    // iBeaconパケット判定
    bool isIBeacon(BLEAdvertisedDevice device) {
      if (device.getManufacturerData().length() < 25) {
        return false;
      }
      if (getCompanyId(device) != 0x00DB){ //} or getCompanyId(device) != 0x00DB) {
        return false;
      }
/*      if (getIBeaconHeader(device) != 0x1502) {
        return false;
      }*/
      return true;
    }

    // CompanyId取得
    unsigned short getCompanyId(BLEAdvertisedDevice device) {
      const unsigned short* pCompanyId = (const unsigned short*)&device
                                         .getManufacturerData()
                                         .c_str()[0];
      return *pCompanyId;
    }

    // iBeacon Header取得
    unsigned short getIBeaconHeader(BLEAdvertisedDevice device) {
      const unsigned short* pHeader = (const unsigned short*)&device
                                      .getManufacturerData()
                                      .c_str()[2];
      return *pHeader;
    }

    // UUID取得
    String getUuid(BLEAdvertisedDevice device) {
      const char* pUuid = &device.getManufacturerData().c_str()[4];
      char uuid[64] = {0};
      sprintf(
        uuid,
        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        pUuid[0], pUuid[1], pUuid[2], pUuid[3], pUuid[4], pUuid[5], pUuid[6], pUuid[7],
        pUuid[8], pUuid[9], pUuid[10], pUuid[11], pUuid[12], pUuid[13], pUuid[14], pUuid[15]
      );
      return String(uuid);
    }

    // TxPower取得
    signed char getTxPower(BLEAdvertisedDevice device) {
      const signed char* pTxPower = (const signed char*)&device
                                    .getManufacturerData()
                                    .c_str()[24];
      return *pTxPower;
    }

    // iBeaconの情報をシリアル出力
    void printIBeacon(BLEAdvertisedDevice device) {
      //gettimeofday(&now, NULL);
      Serial.printf("addr:%s rssi:%d uuid:%s power:%d time:%lf\r\n",
                    device.getAddress().toString().c_str(),
                    device.getRSSI(),
                    getUuid(device).c_str(),
                    *(signed char*)&device.getManufacturerData().c_str()[24],millis() - prev);
    }
};

void setup() {
  M5.begin();
  Serial.begin(115200);
  BLEDevice::init("");
  pinMode(10, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(37, INPUT);
  digitalWrite(10, HIGH);
  M5.Lcd.setRotation(1);
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.print("Ready\n");
  numVals = 0;
  trigger = -70;
}

void loop() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(2, 2);
  M5.Lcd.print("start.\n");
  Serial.println("start.\n");
  BLEScan* scan = BLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new IBeaconAdvertised(), true);
  scan->setActiveScan(true);
  scan->start(300,true);
  //Serial.println("complete.\n");
  //M5.Lcd.print("complete.\n");
}
