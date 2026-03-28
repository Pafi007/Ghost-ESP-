#pragma once

#include "globals.h"

// ============================================================
// Live Beacon Scanner - Promiscuous Mode
// ============================================================

// Channel hopping state
static uint8_t beaconScanChannel = 1;
static unsigned long lastChannelHop = 0;

// Promiscuous callback - called for every received WiFi frame
void IRAM_ATTR beaconSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!beaconScanActive) return;
  if (type != WIFI_PKT_MGMT) return;

  const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  const uint8_t* frame = pkt->payload;
  int len = pkt->rx_ctrl.sig_len;

  if (len < 38) return;

  // Check frame type: management (0x80 = beacon)
  uint8_t frameType = frame[0];
  if (frameType != 0x80) return;

  // Extract BSSID (bytes 16-21)
  const uint8_t* bssid = &frame[16];

  // Extract SSID from tagged parameters (start at byte 36)
  // Tag 0x00 = SSID, followed by length byte, then SSID data
  char ssid[33] = {0};
  int ssidLen = 0;

  if (len > 38) {
    int pos = 36;
    while (pos + 2 <= len) {
      uint8_t tagType = frame[pos];
      uint8_t tagLen = frame[pos + 1];
      if (pos + 2 + tagLen > len) break;

      if (tagType == 0x00) { // SSID tag
        ssidLen = min((int)tagLen, 32);
        memcpy(ssid, &frame[pos + 2], ssidLen);
        ssid[ssidLen] = '\0';
        break;
      }
      pos += 2 + tagLen;
    }
  }

  if (ssidLen == 0) {
    strcpy(ssid, "<Hidden>");
  }

  // Check if printable
  for (int i = 0; i < ssidLen; i++) {
    if (ssid[i] < 32 || ssid[i] > 126) {
      strcpy(ssid, "<Hidden>");
      break;
    }
  }

  int32_t rssi = pkt->rx_ctrl.rssi;
  uint8_t channel = pkt->rx_ctrl.channel;

  // Deduplicate by BSSID - update existing or add new
  for (int i = 0; i < beaconCount; i++) {
    if (memcmp(beaconList[i].bssid, bssid, 6) == 0) {
      // Update RSSI (smoothing: 70% old + 30% new)
      beaconList[i].rssi = (beaconList[i].rssi * 7 + rssi * 3) / 10;
      beaconList[i].lastSeen = millis();
      beaconList[i].channel = channel;
      // Update SSID if we got a real one
      if (strcmp(ssid, "<Hidden>") != 0) {
        strncpy(beaconList[i].ssid, ssid, 32);
        beaconList[i].ssid[32] = '\0';
      }
      return;
    }
  }

  // Add new entry
  if (beaconCount < BEACON_SCAN_MAX) {
    strncpy(beaconList[beaconCount].ssid, ssid, 32);
    beaconList[beaconCount].ssid[32] = '\0';
    memcpy(beaconList[beaconCount].bssid, bssid, 6);
    beaconList[beaconCount].rssi = rssi;
    beaconList[beaconCount].channel = channel;
    beaconList[beaconCount].lastSeen = millis();
    beaconCount++;
  }
}

// Start live beacon scan
void startBeaconScan() {
  beaconCount = 0;
  beaconMenuIndex = 0;
  beaconMenuScroll = 0;
  beaconScanChannel = 1;
  beaconScanActive = true;

  WiFi.mode(WIFI_AP_STA);
  delay(100);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(beaconSnifferCallback);

  // Set promiscuous filter to only management frames
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  esp_wifi_set_promiscuous_filter(&filter);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  lastChannelHop = millis();

  Serial.println("[Beacon] Live scan started");
}

// Stop live beacon scan
void stopBeaconScan() {
  beaconScanActive = false;
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  promiscInitialized = false;
  Serial.printf("[Beacon] Scan stopped. Found %d APs\n", beaconCount);
}

// Hop to next channel - call from loop()
void beaconScanHopChannel() {
  if (!beaconScanActive) return;

  unsigned long now = millis();
  if (now - lastChannelHop >= BEACON_CHANNEL_DWELL_MS) {
    lastChannelHop = now;
    beaconScanChannel++;
    if (beaconScanChannel > BEACON_SCAN_CHANNELS) {
      beaconScanChannel = 1;
    }
    esp_wifi_set_channel(beaconScanChannel, WIFI_SECOND_CHAN_NONE);
  }
}

// Sort beacon list by RSSI (strongest first) - call periodically
void sortBeaconList() {
  for (int i = 0; i < beaconCount - 1; i++) {
    for (int j = 0; j < beaconCount - i - 1; j++) {
      if (beaconList[j].rssi < beaconList[j+1].rssi) {
        BeaconEntry temp = beaconList[j];
        beaconList[j] = beaconList[j+1];
        beaconList[j+1] = temp;
      }
    }
  }
}
