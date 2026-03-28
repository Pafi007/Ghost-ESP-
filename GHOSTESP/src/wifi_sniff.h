#pragma once
#include "globals.h"

// reactiveDeauthSniffer is defined in wifi_attack.h (included before this file)

// ============================================================
// Deauth Monitor Sniffer Callback
// ============================================================
void IRAM_ATTR deauthMonitorSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!deauthMonitorActive) return;
  if (type != WIFI_PKT_MGMT) return;

  const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  const uint8_t* frame = pkt->payload;
  int len = pkt->rx_ctrl.sig_len;

  if (len < 2) return;

  // Check for deauthentication (0xC0) and disassociation (0xA0) frames
  uint8_t frameType = frame[0];
  if (frameType == 0xC0 || frameType == 0xA0) {
    deauthMonitorCount++;
  }
}

// ============================================================
// WiFi Sniffer Callback (for attack mode)
// ============================================================
void IRAM_ATTR wifiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  const uint8_t* frame = pkt->payload;
  int len = pkt->rx_ctrl.sig_len;

  if (currentMode == MODE_WIFI_DEAUTH) {
    reactiveDeauthSniffer(frame, len);
  }
}

void startWifiSniffer() {
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  esp_wifi_set_promiscuous(true);
  promiscInitialized = true;
  esp_wifi_set_promiscuous_rx_cb(wifiSnifferCallback);
  
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_ALL;
  esp_wifi_set_promiscuous_filter(&filter);
  
  if (currentMode == MODE_WIFI_DEAUTH) {
    if (deauthTarget >= 0 && deauthTarget < scanCount) {
      esp_wifi_set_channel(scanResults[deauthTarget].channel, WIFI_SECOND_CHAN_NONE);
    }
  }
}

// ============================================================
// Deauth Monitor Start/Stop
// ============================================================
static uint8_t deauthMonitorChannel = 1;
static unsigned long lastDeauthMonitorHop = 0;

void startDeauthMonitor() {
  deauthMonitorCount = 0;
  deauthMonitorActive = true;
  deauthGraphIndex = 0;
  deauthGraphMax = 1;
  lastDeauthGraphUpdate = millis();
  
  // Clear graph data
  for (int i = 0; i < DEAUTH_GRAPH_WIDTH; i++) {
    deauthGraphData[i] = 0;
  }
  
  deauthMonitorChannel = 1;
  
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  
  esp_wifi_set_promiscuous(true);
  promiscInitialized = true;
  esp_wifi_set_promiscuous_rx_cb(deauthMonitorSnifferCallback);
  
  // Capture management frames only
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  esp_wifi_set_promiscuous_filter(&filter);
  
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  lastDeauthMonitorHop = millis();
  
  Serial.println("[DeauthMon] Monitor started");
}

void stopDeauthMonitor() {
  deauthMonitorActive = false;
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  promiscInitialized = false;
  Serial.printf("[DeauthMon] Stopped. Total detected: %d\n", deauthMonitorCount);
}

// Channel hopping for monitor - call from loop()
void deauthMonitorHopChannel() {
  if (!deauthMonitorActive) return;
  
  unsigned long now = millis();
  if (now - lastDeauthMonitorHop >= 200) { // 200ms dwell time per channel
    lastDeauthMonitorHop = now;
    deauthMonitorChannel++;
    if (deauthMonitorChannel > 13) {
      deauthMonitorChannel = 1;
    }
    esp_wifi_set_channel(deauthMonitorChannel, WIFI_SECOND_CHAN_NONE);
  }
}

// Update graph data - call from loop() every ~500ms
void updateDeauthGraph() {
  if (!deauthMonitorActive) return;
  
  unsigned long now = millis();
  if (now - lastDeauthGraphUpdate >= 500) {
    lastDeauthGraphUpdate = now;
    
    // Store count for this interval, then reset interval counter
    static int lastCount = 0;
    int delta = deauthMonitorCount - lastCount;
    lastCount = deauthMonitorCount;
    
    deauthGraphData[deauthGraphIndex] = delta;
    deauthGraphIndex = (deauthGraphIndex + 1) % DEAUTH_GRAPH_WIDTH;
    
    // Recalculate max for auto-scaling
    deauthGraphMax = 1;
    for (int i = 0; i < DEAUTH_GRAPH_WIDTH; i++) {
      if (deauthGraphData[i] > deauthGraphMax) {
        deauthGraphMax = deauthGraphData[i];
      }
    }
  }
}
