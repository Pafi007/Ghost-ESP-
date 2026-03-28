# GHOSTESP — Multi-Tool v3.0

BLE Spam + WiFi Attacks + Evil Portal + Web UI — everything for ESP32 with OLED 128×64.

---

## Hardware

| Element | Pin/Info |
|---------|----------|
| **Board** | ESP32-WROOM-32U (IPEX antenna) |
| **OLED** | SSD1306 128×64 I2C |
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| **BTN UP** ▲ | GPIO 32 |
| **BTN DOWN** ▼ | GPIO 33 |
| **BTN OK** ● | GPIO 25 |
| **BTN BACK** ◄ | GPIO 26 |


```
  ESP32-WROOM-32U
  ┌─────────────────┐
  │  GPIO 21 (SDA) ─┼── SSD1306 SDA
  │  GPIO 22 (SCL) ─┼── SSD1306 SCL
  │  3.3V          ─┼── SSD1306 VCC
  │  GND           ─┼── SSD1306 GND
  │  GPIO 32       ─┼── BTN UP ──── GND
  │  GPIO 33       ─┼── BTN DOWN ── GND
  │  GPIO 25       ─┼── BTN OK ──── GND
  │  GPIO 26       ─┼── BTN BACK ── GND
  │  IPEX/U.FL     ─┼── Antena 2.4GHz
  └─────────────────┘
```

---

## Interfejs (UI)

### Boot Screen
```
┌────────────────────────┐
│                        │
│        ESP32           │
│         PAFI           │
│                        │
│    Multi-Tool v2       │
│ ██████████████████████ │
└────────────────────────┘
```

### Menu główne (3 kategorie)
```
┌────────────────────────┐
│      ESP32 PAFI        │
│────────────────────────│
│ ┌────────────────────┐ │
│ │█ WiFi            >█│ │  ◄ zaznaczenie
│ └────────────────────┘ │
│ ┌────────────────────┐ │
│ │  Bluetooth        > │ │
│ └────────────────────┘ │
│ ┌────────────────────┐ │
│ │  Others           > │ │
│ └────────────────────┘ │
└────────────────────────┘
```

### Podmenu WiFi
```
┌────────────────────────┐
│█WiFi                  █│
│────────────────────────│
│ █Scan APs           █ │  ◄ zaznaczenie
│   Live Beacon          │
│   Deauth               │
│   Evil Portal          │
│────────────────────────│
│ OK=Go  BACK=Menu       │
└────────────────────────┘
```
(Scrollowalnych 7 opcji: Scan APs, Live Beacon, Deauth, Evil Portal, Beacon PAFI, Beacon Random, Back)

### Podmenu Bluetooth
```
┌────────────────────────┐
│█Bluetooth             █│
│────────────────────────│
│ █Spam All            █ │
│   Spam Samsung         │
│   Spam Windows         │
│   Spam Apple           │
│────────────────────────│
│ OK=Go  BACK=Menu       │
└────────────────────────┘
```
(7 opcji: Spam All/Samsung/Windows/Apple/Android, Stop, Back)

### Podmenu Others
```
┌────────────────────────┐
│█Others                █│
│────────────────────────│
│ █Start Web UI        █ │
│   Stop Web UI          │
│   < Back               │
│────────────────────────│
│ OK=Go  BACK=Menu       │
└────────────────────────┘
```

### Skan WiFi
```
┌────────────────────────┐
│█Scan: 12 APs         █│
│────────────────────────│
│ HomeWifi     -45   6   │
│█Office       -62   1 █│  ◄ zaznaczenie
│ Guest        -71  11   │
│ <Hidden>     -80   3   │
│────────────────────────│
│ OK=Deauth BK=Back      │
└────────────────────────┘
```

### Live Beacon Scan
```
┌────────────────────────┐
│█Live: 23 APs     ... █│
│────────────────────────│
│ NetworkOne   -32   1   │
│ WiFi-5G      -45   6   │
│ Hotspot      -58  11   │
│ MyRouter     -67   3   │
│────────────────────────│
│ BK=Stop  UP/DN=Scrl    │
└────────────────────────┘
```
Ciągłe skanowanie promiscuous, 13 kanałów, RSSI na żywo.

### Evil Portal
```
┌────────────────────────┐
│█Evil Portal           █│
│────────────────────────│
│ AP: Free_WiFi          │
│ Clients: 2  Creds: 1   │
│─────────────────────── │
│ E:user@gmail.com       │  ◄ dane od razu
│ P:haslo123       NEW!  │  ◄ migające NEW!
│────────────────────────│
│    BACK = STOP         │
└────────────────────────┘
```
Tworzy AP "Free_WiFi", wyświetla fake Google login. Przechwycone dane pojawiają się natychmiast na OLED.

### Web UI
```
┌────────────────────────┐
│█Web UI Active         █│
│────────────────────────│
│ SSID: ESP32_PAFI       │
│ IP: 192.168.4.1        │
│ Clients: 1             │
│ Mode: IDLE             │
│────────────────────────│
│    BACK = STOP         │
└────────────────────────┘
```
Panel zarządzania w przeglądarce: scan WiFi, deauth, BLE spam — dark theme.

### Atak aktywny
```
┌────────────────────────┐
│    ATTACK ACTIVE       │
│────────────────────────│
│ BLE Spam ALL           │
│ Pkts: 1423             │
│                        │
│ ██████████░░░░░  anim  │
│────────────────────────│
│    BACK = STOP         │
└────────────────────────┘
```

---

## Nawigacja

| Przycisk | Menu główne | Podmenu | Atak/Ekran |
|----------|-------------|---------|------------|
| **UP** ▲ | Przesuń w górę | Przesuń w górę | — |
| **DOWN** ▼ | Przesuń w dół | Przesuń w dół | — |
| **OK** ● | Wejdź w kategorię | Uruchom opcję | — |
| **BACK** ◄ | — | Wróć do menu | Stop/Wróć |

---

## Funkcje

### WiFi
- **Scan APs** — skanowanie aktywne, do 50 sieci, sortowanie RSSI
- **Live Beacon Scan** — promiscuous mode, 13 kanałów, 150ms dwell, RSSI na żywo
- **Deauth** — deautentykacja wybranego AP
- **Evil Portal** — AP "Free_WiFi" + fałszywy Google login + przechwytywanie haseł
- **Beacon PAFI x25** — 25 fake sieci "GOT HACKED BY PAFI"
- **Beacon Random** — 25 sieci z losowymi SSID

### Bluetooth (BLE Spam)
- **Spam All/Samsung/Windows/Apple/Android** — fałszywe parowania BLE

### Others
- **Web UI** — panel zarządzania na `192.168.4.1` (dark theme)

---

## Struktura plików

```
src/
├── main.cpp              — setup/loop, input handler (~300 linii)
├── config.h              — piny, stałe
├── globals.h             — enumy, struktury, extern
├── buttons.h             — odczyt przycisków
├── ble_spam.h            — BLE payloady + spam
├── wifi_scan.h           — ulepszony scan AP
├── wifi_attack.h         — deauth, beacon spam
├── beacon_scan.h         — live beacon scanner
├── evil_portal.h         — captive portal
├── evil_portal_html.h    — fake Google login HTML
├── webui.h               — serwer Web UI
├── webui_html.h          — panel HTML
└── ui.h                  — rysowanie menu/ekranów
```

---

## Kompilacja

```bash
cd esp32_pafi_oled
pio run                    # kompiluj
pio run -t upload          # wgraj na ESP32
pio device monitor -b 115200  # serial monitor
```

## Flash gotowego .bin

```bash
esptool.py --chip esp32 --baud 921600 write_flash 0x0 ESP32_PAFI_V2_FULL.bin
```
