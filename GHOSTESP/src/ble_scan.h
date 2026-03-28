#pragma once
#include "globals.h"

BLEScan* pBLEScan = nullptr;

#define MAX_UNIQUE_BLE 50
std::vector<std::string> seenBleMacs;

class MyBLEAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (currentMode == MODE_BLE_SCAN) {
        std::string mac = advertisedDevice.getAddress().toString().c_str();
        bool isNew = true;
        for (const auto& m : seenBleMacs) {
          if (m == mac) { isNew = false; break; }
        }
        if (isNew) {
          if (seenBleMacs.size() < MAX_UNIQUE_BLE) seenBleMacs.push_back(mac);
          bleDeviceCount++;
        }

        strncpy(lastBleMac, advertisedDevice.getAddress().toString().c_str(), 17);
        lastBleMac[17] = '\0';
        if (advertisedDevice.haveName()) {
          strncpy(lastBleName, advertisedDevice.getName().c_str(), 32);
          lastBleName[32] = '\0';
        } else {
          strcpy(lastBleName, "<Unknown>");
        }
      }
    }
};

void startBLEScanner() {
  if (!bleInitialized) {
    BLEDevice::init("");
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    bleInitialized = true;
  }
  bleDeviceCount = 0;
  seenBleMacs.clear();
  strcpy(lastBleMac, ""); strcpy(lastBleName, "");
  
  if (pBLEScan == nullptr) {
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyBLEAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(true); 
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99); 
  }
  pBLEScan->start(0, nullptr, false); // continuous
}

void stopBLEScanner() {
  if (pBLEScan != nullptr) {
    pBLEScan->stop();
    pBLEScan->clearResults();
  }
}
