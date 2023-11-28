#include <Arduino.h>
#include "M5Atom.h"

#include "sys/time.h"
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"

#define GPIO_DEEP_SLEEP_DURATION     250  // sleep x milli seconds and then wake up
#define LED_BUILTIN   
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

// Variables will change:
bool ledState = 0;  // ledState used to set the LED
unsigned long previousMillis = 0;  // will store last time LED was updated
long interval = 1000;  // interval at which to blink (milliseconds)

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLEAdvertising *pAdvertising;   // BLE Advertisement type
struct timeval now;
#define BEACON_UUID "5f79e400-9e6a-4eff-8255-28246e1891a8" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)

void setBeacon() {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0xDB00); // UNIQUE ID FOR SWIMMING BEACON
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount & 0xFFFF);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04

  std::string strServiceData = "";

  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData();
  oAdvertisementData.addData(strServiceData);

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
}

void setup() {
  M5.begin(true, false, true);  // Init Atom-Matrix(Initialize serial port, LED).
  Serial.begin(115200);
  M5.dis.drawpix(0, 0x00ff00);  // Light the LED with the specified RGB color

  gettimeofday(&now, NULL);
  Serial.printf("start ESP32 %d\n", bootcount++);
  Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n", now.tv_sec, now.tv_sec - last);
  last = now.tv_sec;

  // Create the BLE Device
  BLEDevice::init("ESP32 as iBeacon");
  //esp_bredr_tx_power_set(ESP_PWR_LVL_N12, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); 
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9);
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage
  pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::startAdvertising();
  setBeacon();
  // Start advertising
  pAdvertising->start();
  Serial.println("Advertizing started...");
//  delay(100000);
//  pAdvertising->stop();
//  Serial.printf("enter deep sleep\n");
//  esp_deep_sleep(1000LL * GPIO_DEEP_SLEEP_DURATION);
//  Serial.printf("in deep sleep\n");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState) {
        M5.dis.drawpix(0, 0x000000);  // Light the LED with the specified RGB color
        interval = 10000;
    } else {
        M5.dis.drawpix(0, 0x0000ff);  // Light the LED with the specified RGB color
        interval = 250;
    }
    ledState = !ledState;
  }
}