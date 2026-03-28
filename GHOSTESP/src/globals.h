#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

extern "C" {
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
  esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
}

#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <BLEUtils.h>
#include <esp_bt.h>

#include "config.h"

// ============================================================
// Enums
// ============================================================

// Main menu categories
enum MenuCategory {
  CAT_NONE = -1,
  CAT_WIFI = 0,
  CAT_BT = 1,
  CAT_OTHERS = 2,
  CAT_COUNT = 3
};

enum Screen {
  SCREEN_MAIN_MENU,    // 3 big categories
  SCREEN_SUBMENU,      // items inside category
  SCREEN_SCAN_RESULTS,
  SCREEN_RUNNING,
  SCREEN_BEACON_SCAN,
  SCREEN_EVIL_PORTAL,
  SCREEN_WEBUI,
  SCREEN_TAMAGOTCHI,
  SCREEN_DEAUTH_MONITOR  // NEW: live deauth packet monitor
};

enum AttackMode {
  MODE_IDLE,
  MODE_BLE_ALL, MODE_BLE_SAMSUNG, MODE_BLE_WINDOWS, MODE_BLE_APPLE, MODE_BLE_ANDROID,
  MODE_BLE_SCAN, MODE_WIFI_DEAUTH, MODE_WIFI_BEACON, MODE_WIFI_BEACON_RANDOM,
  MODE_BEACON_SCAN, MODE_EVIL_PORTAL, MODE_TAMAGOTCHI,
  MODE_DEAUTH_MONITOR  // NEW
};

enum Button {
  BTN_NONE = 0,
  BTN_UP,
  BTN_DOWN,
  BTN_OK,
  BTN_BACK
};

// ============================================================
// Scan Result
// ============================================================
typedef struct {
  String ssid;
  int32_t rssi;
  uint8_t bssid[6];
  uint8_t channel;
} ScanResult;

// ============================================================
// Beacon Scan Entry
// ============================================================
typedef struct {
  char ssid[33];
  uint8_t bssid[6];
  int32_t rssi;
  uint8_t channel;
  unsigned long lastSeen;
} BeaconEntry;

// ============================================================
// Evil Portal Credential
// ============================================================
typedef struct {
  String email;
  String password;
  unsigned long capturedAt;
} CapturedCred;

// ============================================================
// Extern Globals
// ============================================================
extern Adafruit_SSD1306 display;

// UI state
extern Screen currentScreen;
extern MenuCategory currentCategory;
extern int mainMenuIndex;       // 0-2 for main menu
extern int subMenuIndex;        // item in submenu
extern int subMenuScroll;

// Menu animation (integer pixel position)
extern int selY;        // current highlight Y
extern int selTargetY;  // target highlight Y


// Attack state
extern AttackMode currentMode;
extern BLEAdvertising *advertising;
extern uint32_t packetCount;
extern unsigned long lastLedToggle;
extern bool bleInitialized;
extern bool promiscInitialized;
extern volatile bool radioTxBusy;
extern bool displayNeedsUpdate;
extern unsigned long attackStartTime;

// WiFi scan
extern ScanResult scanResults[];
extern int scanCount;
extern int deauthTarget;
extern int deauthPackets;
extern int scanMenuIndex;
extern int scanMenuScroll;

// WiFi beacon channel rotation
extern int wifiChannels[];
extern int wifiChIdx;

// Beacon scanner
extern BeaconEntry beaconList[];
extern int beaconCount;
extern int beaconMenuIndex;
extern int beaconMenuScroll;
extern volatile bool beaconScanActive;

// Evil Portal
extern CapturedCred capturedCreds[];
extern int credCount;
extern int credMenuIndex;
extern volatile bool newCredCaptured;

// Button
extern unsigned long lastBtnPress;
extern unsigned long lastDisplayRefresh;

// New Scanners State
extern volatile int bleDeviceCount;
extern char lastBleMac[18];
extern char lastBleName[33];

// ============================================================
// Deauth Monitor Globals
// ============================================================
#define DEAUTH_GRAPH_WIDTH 100
#define DEAUTH_GRAPH_HEIGHT 30

extern volatile int deauthMonitorCount;     // total deauth packets detected
extern int deauthGraphData[];               // circular buffer of deauth counts per interval
extern int deauthGraphIndex;                // current position in circular buffer
extern unsigned long lastDeauthGraphUpdate; // last time we pushed a value
extern volatile bool deauthMonitorActive;   // is deauth monitor running
extern int deauthGraphMax;                  // max value in current graph for auto-scaling

// ============================================================
// Tamagotchi Ghost State
// ============================================================
extern int tgHunger;           // 0-100
extern int tgHappy;            // 0-100
extern int tgEnergy;           // 0-100
extern int tgAction;           // 0=none, 1=feed, 2=play, 3=sleep
extern unsigned long tgActionTime;
extern unsigned long tgLastDecay;
extern int tgAge;              // in minutes
extern unsigned long tgBornTime;
extern int tgFeedCount;
extern int tgPlayCount;
