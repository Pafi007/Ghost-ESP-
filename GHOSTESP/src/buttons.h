#pragma once

#include "globals.h"

// ============================================================
// Button Reading with Debounce
// ============================================================
Button readButton() {
  if (millis() - lastBtnPress < DEBOUNCE_MS) return BTN_NONE;
  
  if (digitalRead(PIN_BTN_UP) == LOW)   { lastBtnPress = millis(); return BTN_UP; }
  if (digitalRead(PIN_BTN_DOWN) == LOW) { lastBtnPress = millis(); return BTN_DOWN; }
  if (digitalRead(PIN_BTN_OK) == LOW)   { lastBtnPress = millis(); return BTN_OK; }
  if (digitalRead(PIN_BTN_BACK) == LOW) { lastBtnPress = millis(); return BTN_BACK; }
  
  return BTN_NONE;
}
