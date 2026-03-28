#pragma once

#include "globals.h"

// ============================================================
// Improved WiFi AP Scan
// ============================================================
void doWiFiScan() {
  scanCount = 0;

  Serial.println("[WiFi] Preparing scan...");

  // --- CRITICAL: Disable promiscuous mode before scanning ---
  // Promiscuous mode (used by beacon scan & deauth) blocks normal AP scanning
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_set_promiscuous(false);
  promiscInitialized = false;
  delay(50);

  // --- Reinitialize WiFi in AP+STA mode for scanning ---
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP_STA);
  delay(200);

  Serial.println("[WiFi] Starting AP scan...");

  // Try Arduino API first (more reliable after mode changes)
  int found = WiFi.scanNetworks(false, true, false, 300);
  Serial.printf("[WiFi] scanNetworks returned: %d\n", found);

  if (found > 0) {
    int count = min(found, MAX_SCAN_RESULTS);
    for (int i = 0; i < count; i++) {
      String ssid = WiFi.SSID(i);
      if (ssid.length() == 0) ssid = "<Hidden>";

      scanResults[i].ssid = ssid;
      scanResults[i].rssi = WiFi.RSSI(i);
      scanResults[i].channel = WiFi.channel(i);
      memcpy(scanResults[i].bssid, WiFi.BSSID(i), 6);
    }
    scanCount = count;
    WiFi.scanDelete();
    Serial.printf("[WiFi] Scan found %d APs\n", scanCount);
    return;
  }

  // Fallback: low-level ESP-IDF scan API
  Serial.println("[WiFi] Arduino scan failed, trying ESP-IDF API...");

  wifi_scan_config_t scanConf;
  memset(&scanConf, 0, sizeof(scanConf));
  scanConf.ssid = NULL;
  scanConf.bssid = NULL;
  scanConf.channel = 0;  // all channels
  scanConf.show_hidden = true;
  scanConf.scan_type = WIFI_SCAN_TYPE_ACTIVE;
  scanConf.scan_time.active.min = 120;
  scanConf.scan_time.active.max = 500;

  esp_err_t r = esp_wifi_scan_start(&scanConf, true);
  if (r != ESP_OK) {
    Serial.printf("[WiFi] ESP-IDF Scan failed: 0x%x\n", r);
    return;
  }

  uint16_t apCount = 0;
  esp_wifi_scan_get_ap_num(&apCount);
  Serial.printf("[WiFi] ESP-IDF found %d APs\n", apCount);

  if (apCount == 0) {
    return;
  }

  // Get all results
  wifi_ap_record_t* apRecords = new wifi_ap_record_t[apCount];
  uint16_t got = apCount;
  esp_err_t err = esp_wifi_scan_get_ap_records(&got, apRecords);

  if (err != ESP_OK || got == 0) {
    delete[] apRecords;
    return;
  }

  // Sort by RSSI (strongest first) - simple bubble sort
  for (int i = 0; i < got - 1; i++) {
    for (int j = 0; j < got - i - 1; j++) {
      if (apRecords[j].rssi < apRecords[j+1].rssi) {
        wifi_ap_record_t temp = apRecords[j];
        apRecords[j] = apRecords[j+1];
        apRecords[j+1] = temp;
      }
    }
  }

  // Copy to our array (capped at MAX_SCAN_RESULTS)
  int count = min((int)got, MAX_SCAN_RESULTS);
  for (int i = 0; i < count; i++) {
    char ssidBuf[33];
    strncpy(ssidBuf, (const char*)apRecords[i].ssid, 32);
    ssidBuf[32] = '\0';

    String ssid = String(ssidBuf);
    if (ssid.length() == 0) ssid = "<Hidden>";

    scanResults[i].ssid = ssid;
    scanResults[i].rssi = apRecords[i].rssi;
    scanResults[i].channel = apRecords[i].primary;
    memcpy(scanResults[i].bssid, apRecords[i].bssid, 6);
  }

  scanCount = count;
  delete[] apRecords;

  Serial.printf("[WiFi] Scan found %d APs\n", scanCount);
}
