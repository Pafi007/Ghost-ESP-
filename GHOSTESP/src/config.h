#pragma once

// ============================================================
// Hardware Pins
// ============================================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C

#define PIN_SDA       21
#define PIN_SCL       22
#define PIN_BTN_UP    32
#define PIN_BTN_DOWN  33
#define PIN_BTN_OK    25
#define PIN_BTN_BACK  26
#define BLUE_LED_PIN  2

// ============================================================
// Config
// ============================================================
#define MAX_TX_POWER       ESP_PWR_LVL_P9
#define MAX_SCAN_RESULTS   50
#define BEACON_COUNT       25
#define DEBOUNCE_MS        180
#define LONG_PRESS_MS      600
#define VISIBLE_ITEMS      4

// Display refresh intervals
#define DISPLAY_REFRESH_MS_IDLE   100
#define DISPLAY_REFRESH_MS_ATTACK 500

// Beacon scanner
#define BEACON_SCAN_MAX    64
#define BEACON_CHANNEL_DWELL_MS 150
#define BEACON_SCAN_CHANNELS 13

// Evil Portal
#define EVIL_PORTAL_SSID   "Free_WiFi"
#define EVIL_PORTAL_MAX_CREDS 10

// Web UI
#define WEBUI_AP_SSID      "GhostWave"
#define WEBUI_AP_PASS      ""
