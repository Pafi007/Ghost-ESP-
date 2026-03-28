/*
 * GhostWave - Multi-Tool v3.0
 * Board: ESP32-WROOM-32U (IPEX/U.FL antenna)
 * Display: SSD1306 128x64 I2C OLED with 4 buttons
 * 
 * Features: BLE Spam, WiFi Scan/Deauth/Beacon, Live Beacon Scanner,
 *           Evil Portal, Deauth Monitor, Tamagotchi
 * 
 * PINOUT:
 * SSD1306 OLED: SDA=GPIO21, SCL=GPIO22
 * Buttons (active LOW): UP=GPIO32, DOWN=GPIO33, OK=GPIO25, BACK=GPIO26
 * LED: GPIO2 (built-in blue)
 */

#include "config.h"
#include "globals.h"

// ============================================================
// Global Variable Definitions
// ============================================================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// UI state
Screen currentScreen = SCREEN_MAIN_MENU;
MenuCategory currentCategory = CAT_NONE;
int mainMenuIndex = 0;
int subMenuIndex = 0;
int subMenuScroll = 0;

// Menu animation
int selY = 13;
int selTargetY = 13;

// Attack state
AttackMode currentMode = MODE_IDLE;
BLEAdvertising *advertising = nullptr;
uint32_t packetCount = 0;
unsigned long lastLedToggle = 0;
bool bleInitialized = false;
bool promiscInitialized = false;
volatile bool radioTxBusy = false;
bool displayNeedsUpdate = true;
unsigned long attackStartTime = 0;

// WiFi scan
ScanResult scanResults[MAX_SCAN_RESULTS];
int scanCount = 0;
int deauthTarget = -1;
int deauthPackets = 0;
int scanMenuIndex = 0;
int scanMenuScroll = 0;

// WiFi beacon channel rotation
int wifiChannels[] = {1, 6, 11};
int wifiChIdx = 0;

// Beacon scanner
BeaconEntry beaconList[BEACON_SCAN_MAX];
int beaconCount = 0;
int beaconMenuIndex = 0;
int beaconMenuScroll = 0;
volatile bool beaconScanActive = false;

// Evil Portal
CapturedCred capturedCreds[EVIL_PORTAL_MAX_CREDS];
int credCount = 0;
int credMenuIndex = 0;
volatile bool newCredCaptured = false;

// Button
unsigned long lastBtnPress = 0;
unsigned long lastDisplayRefresh = 0;

// BLE Scanner
volatile int bleDeviceCount = 0;
char lastBleMac[18] = {0};
char lastBleName[33] = {0};

// Deauth Monitor
volatile int deauthMonitorCount = 0;
int deauthGraphData[DEAUTH_GRAPH_WIDTH] = {0};
int deauthGraphIndex = 0;
unsigned long lastDeauthGraphUpdate = 0;
volatile bool deauthMonitorActive = false;
int deauthGraphMax = 1;

// Tamagotchi Ghost
int tgHunger = 80;
int tgHappy = 80;
int tgEnergy = 80;
int tgAction = 0;
unsigned long tgActionTime = 0;
unsigned long tgLastDecay = 0;
int tgAge = 0;
unsigned long tgBornTime = 0;
int tgFeedCount = 0;
int tgPlayCount = 0;

// ============================================================
// Include Modules (after globals are defined)
// ============================================================
#include "buttons.h"
#include "ble_spam.h"
#include "wifi_attack.h"
#include "wifi_scan.h"
#include "beacon_scan.h"
#include "wifi_sniff.h"
#include "evil_portal.h"
#include "ble_scan.h"
#include "ui.h"

// ============================================================
// Attack Control
// ============================================================
void stopAttack() {
  if (currentMode == MODE_BEACON_SCAN) stopBeaconScan();
  if (currentMode == MODE_EVIL_PORTAL) stopEvilPortal();
  if (currentMode == MODE_BLE_SCAN) stopBLEScanner();
  if (currentMode == MODE_DEAUTH_MONITOR) stopDeauthMonitor();
  if (currentMode == MODE_WIFI_DEAUTH) {
    esp_wifi_set_promiscuous_rx_cb(NULL);
    esp_wifi_set_promiscuous(false);
    promiscInitialized = false;
    if (currentMode == MODE_WIFI_DEAUTH) {
      WiFi.softAPdisconnect(true);
    }
  }

  currentMode = MODE_IDLE;
  if (advertising) advertising->stop();
  digitalWrite(BLUE_LED_PIN, LOW);
  packetCount = 0;
  deauthPackets = 0;
  
  // Always ensure WiFi is back to a safe idle state
  WiFi.mode(WIFI_AP_STA);
  
  currentScreen = SCREEN_MAIN_MENU;
}

void startAttack(AttackMode mode) {
  if (currentMode != MODE_IDLE) stopAttack();
  currentMode = mode;
  packetCount = 0;
  deauthPackets = 0;
  attackStartTime = millis();

  if (mode == MODE_WIFI_DEAUTH) {
    startWifiSniffer();
    if (deauthTarget >= 0 && deauthTarget < scanCount) {
      rsnk_attack_method_rogueap(scanResults[deauthTarget].bssid, scanResults[deauthTarget].ssid.c_str(), scanResults[deauthTarget].channel);
    }
  } else if (mode == MODE_WIFI_BEACON || mode == MODE_WIFI_BEACON_RANDOM) {
    ensurePromisc();
  } else if (mode == MODE_BLE_SCAN) {
    startBLEScanner();
  } else if (mode == MODE_DEAUTH_MONITOR) {
    startDeauthMonitor();
    currentScreen = SCREEN_DEAUTH_MONITOR;
    return;  // Don't set SCREEN_RUNNING
  } else if (mode >= MODE_BLE_ALL && mode <= MODE_BLE_ANDROID) {
    // Crucial for BLE SPAM: Turn off WiFi completely
    WiFi.mode(WIFI_MODE_NULL);
    delay(100);
  }

  currentScreen = SCREEN_RUNNING;
}



// ============================================================
// Submenu Action Handler
// ============================================================
void handleSubMenuAction() {
  if (currentCategory == CAT_BT) {
    switch(subMenuIndex) {
      case 0: startAttack(MODE_BLE_ALL); break;
      case 1: startAttack(MODE_BLE_SAMSUNG); break;
      case 2: startAttack(MODE_BLE_WINDOWS); break;
      case 3: startAttack(MODE_BLE_APPLE); break;
      case 4: startAttack(MODE_BLE_ANDROID); break;
      case 5: startAttack(MODE_BLE_SCAN); break;
      case 6: stopAttack(); break;
      case 7: // Back
        currentScreen = SCREEN_MAIN_MENU;
        break;
    }
  } else if (currentCategory == CAT_WIFI) {
    switch(subMenuIndex) {
      case 0: // Scan APs
        drawScanningScreen();
        doWiFiScan();
        scanMenuIndex = 0;
        scanMenuScroll = 0;
        currentScreen = SCREEN_SCAN_RESULTS;
        break;
      case 1: // Live Beacon Scan
        startBeaconScan();
        currentMode = MODE_BEACON_SCAN;
        currentScreen = SCREEN_BEACON_SCAN;
        break;
      case 2: // Deauth Target
        if (scanCount == 0) {
          drawScanningScreen();
          doWiFiScan();
        }
        scanMenuIndex = 0;
        scanMenuScroll = 0;
        currentScreen = SCREEN_SCAN_RESULTS;
        break;
      case 3: // Evil Portal
        startEvilPortal();
        currentMode = MODE_EVIL_PORTAL;
        currentScreen = SCREEN_EVIL_PORTAL;
        break;
      case 4: startAttack(MODE_WIFI_BEACON); break;
      case 5: startAttack(MODE_WIFI_BEACON_RANDOM); break;
      case 6: // Deauth Monitor
        startAttack(MODE_DEAUTH_MONITOR);
        break;
      case 7: // Back
        currentScreen = SCREEN_MAIN_MENU;
        break;
    }
  } else if (currentCategory == CAT_OTHERS) {
    switch(subMenuIndex) {
      case 0: // Tamagotchi
        if (currentMode == MODE_IDLE) {
          currentMode = MODE_TAMAGOTCHI;
          if (tgBornTime == 0) tgBornTime = millis();
          tgLastDecay = millis();
        }
        currentScreen = SCREEN_TAMAGOTCHI;
        break;
      case 1: // Back
        currentScreen = SCREEN_MAIN_MENU;
        break;
    }
  }
}

// ============================================================
// Input Handler
// ============================================================
void handleInput(Button btn) {
  switch(currentScreen) {
    case SCREEN_MAIN_MENU:
      switch(btn) {
        case BTN_UP:
          if (mainMenuIndex > 0) mainMenuIndex--;
          else mainMenuIndex = CAT_COUNT - 1;
          break;
        case BTN_DOWN:
          if (mainMenuIndex < CAT_COUNT - 1) mainMenuIndex++;
          else mainMenuIndex = 0;
          break;
        case BTN_OK:
          currentCategory = (MenuCategory)mainMenuIndex;
          subMenuIndex = 0;
          subMenuScroll = 0;
          currentScreen = SCREEN_SUBMENU;
          break;
        default: break;
      }
      break;

    case SCREEN_SUBMENU:
      {
        int itemCount = getSubMenuCount();
        switch(btn) {
          case BTN_UP:
            if (subMenuIndex > 0) subMenuIndex--;
            else subMenuIndex = itemCount - 1;
            break;
          case BTN_DOWN:
            if (subMenuIndex < itemCount - 1) subMenuIndex++;
            else subMenuIndex = 0;
            break;
          case BTN_OK:
            handleSubMenuAction();
            break;
          case BTN_BACK:
            currentScreen = SCREEN_MAIN_MENU;
            break;
          default: break;
        }
      }
      break;

    case SCREEN_SCAN_RESULTS:
      switch(btn) {
        case BTN_UP:
          if (scanMenuIndex > 0) scanMenuIndex--;
          else scanMenuIndex = scanCount - 1;
          break;
        case BTN_DOWN:
          if (scanMenuIndex < scanCount - 1) scanMenuIndex++;
          else scanMenuIndex = 0;
          break;
        case BTN_OK:
          if (scanCount > 0 && scanMenuIndex >= 0 && scanMenuIndex < scanCount) {
            deauthTarget = scanMenuIndex;
            startAttack(MODE_WIFI_DEAUTH);
          }
          break;
        case BTN_BACK:
          currentScreen = SCREEN_SUBMENU;
          break;
        default: break;
      }
      break;

    case SCREEN_RUNNING:
      if (btn == BTN_BACK || btn == BTN_OK) stopAttack();
      break;

    case SCREEN_BEACON_SCAN:
      switch(btn) {
        case BTN_UP:
          if (beaconMenuIndex > 0) beaconMenuIndex--;
          break;
        case BTN_DOWN:
          if (beaconMenuIndex < beaconCount - 1) beaconMenuIndex++;
          break;
        case BTN_BACK:
          stopBeaconScan();
          currentMode = MODE_IDLE;
          currentScreen = SCREEN_SUBMENU;
          break;
        default: break;
      }
      break;

    case SCREEN_EVIL_PORTAL:
      if (btn == BTN_BACK) {
        stopEvilPortal();
        currentMode = MODE_IDLE;
        currentScreen = SCREEN_SUBMENU;
      }
      break;

    case SCREEN_DEAUTH_MONITOR:
      if (btn == BTN_BACK) {
        stopDeauthMonitor();
        currentMode = MODE_IDLE;
        currentScreen = SCREEN_SUBMENU;
      }
      break;

    case SCREEN_TAMAGOTCHI:
      if (btn == BTN_BACK) {
        if (currentMode == MODE_TAMAGOTCHI) currentMode = MODE_IDLE;
        currentScreen = SCREEN_SUBMENU;
      } else if (btn == BTN_UP && tgAction == 0) {
        // Feed the ghost
        tgAction = 1;
        tgActionTime = millis();
        tgHunger = constrain(tgHunger + 20, 0, 100);
        tgFeedCount++;
      } else if (btn == BTN_DOWN && tgAction == 0) {
        // Play with ghost
        tgAction = 2;
        tgActionTime = millis();
        tgHappy = constrain(tgHappy + 20, 0, 100);
        tgPlayCount++;
      } else if (btn == BTN_OK && tgAction == 0) {
        // Put ghost to sleep
        tgAction = 3;
        tgActionTime = millis();
        tgEnergy = constrain(tgEnergy + 25, 0, 100);
      }
      break;
  }
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("[*] GhostWave v3.0 booting...");

  pinMode(BLUE_LED_PIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite(BLUE_LED_PIN, HIGH); delay(100);
    digitalWrite(BLUE_LED_PIN, LOW); delay(100);
  }

  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  pinMode(PIN_BTN_OK, INPUT_PULLUP);
  pinMode(PIN_BTN_BACK, INPUT_PULLUP);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      while(1) {
        digitalWrite(BLUE_LED_PIN, HIGH); delay(50);
        digitalWrite(BLUE_LED_PIN, LOW); delay(50);
      }
    }
  }
  Serial.println("[*] OLED OK");
  display.clearDisplay();
  display.display();

  showBootScreen();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("GhostWave", "", 1, 1);
  WiFi.disconnect();
  delay(300);

  Serial.println("[*] GhostWave v3.0 ready!");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  // 1. Buttons
  Button btn = readButton();
  if (btn != BTN_NONE) {
    handleInput(btn);
    displayNeedsUpdate = true;
  }

  // 2. Network services
  processEvilPortal();

  // 3. Beacon scan hop
  if (beaconScanActive) {
    beaconScanHopChannel();
    static unsigned long lastSort = 0;
    if (millis() - lastSort > 2000) {
      sortBeaconList();
      lastSort = millis();
    }
  }

  // 3b. Deauth monitor hop + graph update
  if (deauthMonitorActive) {
    deauthMonitorHopChannel();
    updateDeauthGraph();
  }

  // 3c. Tamagotchi ghost stat decay
  if (currentMode == MODE_TAMAGOTCHI) {
    unsigned long now2 = millis();
    // Decay stats every 15 seconds
    if (now2 - tgLastDecay > 15000) {
      tgHunger = constrain(tgHunger - 2, 0, 100);
      tgHappy = constrain(tgHappy - 1, 0, 100);
      tgEnergy = constrain(tgEnergy - 1, 0, 100);
      tgAge = (now2 - tgBornTime) / 60000;
      tgLastDecay = now2;
    }
    // Clear action after 1.5 seconds
    if (tgAction != 0 && now2 - tgActionTime > 1500) {
      tgAction = 0;
    }
  }

  // 4. Force update when new credential captured
  if (newCredCaptured) {
    displayNeedsUpdate = true;
  }

  // 5. Display refresh
  unsigned long now = millis();
  int refreshInterval = (currentMode == MODE_IDLE) ? DISPLAY_REFRESH_MS_IDLE : DISPLAY_REFRESH_MS_ATTACK;
  if (currentScreen == SCREEN_EVIL_PORTAL) refreshInterval = 200;
  if (currentScreen == SCREEN_TAMAGOTCHI) refreshInterval = 50;  // Smoother animations
  if (currentScreen == SCREEN_DEAUTH_MONITOR) refreshInterval = 100; // Fast graph update

  if (!radioTxBusy && (displayNeedsUpdate || (now - lastDisplayRefresh >= (unsigned long)refreshInterval))) {
    lastDisplayRefresh = now;
    displayNeedsUpdate = false;

    switch(currentScreen) {
      case SCREEN_MAIN_MENU:      drawMainMenu(); break;
      case SCREEN_SUBMENU:        drawSubMenu(); break;
      case SCREEN_SCAN_RESULTS:   drawScanResults(); break;
      case SCREEN_RUNNING:        drawRunningScreen(); break;
      case SCREEN_BEACON_SCAN:    drawBeaconScanScreen(); break;
      case SCREEN_EVIL_PORTAL:    drawEvilPortalScreen(); break;
      case SCREEN_TAMAGOTCHI:     drawTamagotchiScreen(); break;
      case SCREEN_DEAUTH_MONITOR: drawDeauthMonitorScreen(); break;
    }
  }

  // 6. Attack execution
  switch (currentMode) {
    case MODE_BLE_ALL:     spamAll(); break;
    case MODE_BLE_SAMSUNG: spamSamsung(); break;
    case MODE_BLE_WINDOWS: spamWindows(); break;
    case MODE_BLE_APPLE:   spamApple(); break;
    case MODE_BLE_ANDROID: spamAndroid(); break;
    case MODE_WIFI_BEACON:
      radioTxBusy = true; wifiBeaconSpamGW(); radioTxBusy = false; delay(50); break;
    case MODE_WIFI_BEACON_RANDOM:
      radioTxBusy = true; wifiBeaconRandom(); radioTxBusy = false; delay(50); break;
    case MODE_WIFI_DEAUTH:
      radioTxBusy = true;
      if (deauthTarget >= 0 && deauthTarget < scanCount) {
        rsnk_attack_method_broadcast(scanResults[deauthTarget].bssid);
      } else if (deauthTarget == -1 && scanCount > 0) {
        for (int i = 0; i < scanCount; i++) {
          rsnk_attack_method_broadcast(scanResults[i].bssid);
          delay(1);
        }
      }
      radioTxBusy = false;
      delay(50);
      
      if (deauthPackets > 0) {
        static int lastDeauthPackets = 0;
        if (deauthPackets > lastDeauthPackets) {
          digitalWrite(BLUE_LED_PIN, !digitalRead(BLUE_LED_PIN));
          lastDeauthPackets = deauthPackets;
        }
      }
      break;
    default: break;
  }

  // 7. Idle LED breathing effect
  if (currentMode == MODE_IDLE) {
    if (now - lastLedToggle > 1000) {
      digitalWrite(BLUE_LED_PIN, !digitalRead(BLUE_LED_PIN));
      lastLedToggle = now;
    }
  }

  yield();
  delay(5);
}
