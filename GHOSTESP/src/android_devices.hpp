#pragma once

#include <stdint.h>

// Comprehensive Android Fast Pair Model IDs for BLE spam
const uint32_t ANDROID_MODEL_IDS[] PROGMEM = {
    // Google Pixel Buds
    0x000047, // Google Pixel Buds
    0x470000, // Google Pixel Buds
    0x0600F0, // Google Pixel Buds Pro
    0x0600F1, // Google Pixel Buds Pro 2
    0xD800FE, // Google Pixel Buds A-Series

    // Google / Pixel devices
    0x03F5D4, // Google (Writing API Key)
    0x8D5B67, // Pixel 90c
    0x989D0A, // Set up your new Pixel
    0x821F66, // Pixel 6
    0xCD8256, // Pixel 7
    0x3B41C2, // Pixel 7a
    0x7B2732, // Pixel 8
    0x5BA2E0, // Pixel 8 Pro
    0x0B5996, // Pixel 8a
    0x9B29A6, // Pixel 9

    // Samsung Galaxy Buds
    0x8D13B9, // Samsung Galaxy Buds
    0xA7C62B, // Samsung Galaxy Buds2
    0xA7C62C, // Samsung Galaxy Buds2 Pro
    0xA7C62D, // Samsung Galaxy Buds FE
    0xA7C62E, // Samsung Galaxy Buds3
    0xA7C62F, // Samsung Galaxy Buds3 Pro
    0x99F098, // Samsung Galaxy S22 Ultra
    0xA45EF5, // Samsung Galaxy S23
    0xA45EF6, // Samsung Galaxy S23 Ultra
    0xA45EF7, // Samsung Galaxy S24
    0xA45EF8, // Samsung Galaxy S24 Ultra
    0xC14CA0, // Samsung Galaxy Watch 5
    0xD1698E, // Samsung Galaxy Watch 6
    0xF52494, // Samsung Galaxy Ring

    // Sony
    0x04AFF8, // Sony WF-1000XM4
    0x04B002, // Sony WH-1000XM5
    0x04B003, // Sony WF-1000XM5
    0x04B004, // Sony LinkBuds S
    0x04B005, // Sony LinkBuds
    0xC37FBE, // Sony WH-1000XM4

    // JBL
    0x02AA91, // JBL Tune Flex
    0x02AA92, // JBL Tune Buds
    0x02AA93, // JBL Live Pro 2
    0x02AA94, // JBL Tune Beam
    0x02AA95, // JBL Tour Pro 2
    0x02AA96, // JBL Endurance Race
    0x02AA97, // JBL Vibe 200TWS
    0x02AA98, // JBL Vibe Beam
    0x02AA99, // JBL Live Flex
    0x02AA9A, // JBL Wave Flex
    0x02AB60, // JBL Flip 6
    0x02AB61, // JBL Charge 5
    0x02AB62, // JBL Xtreme 3
    0x02AB63, // JBL Clip 4

    // Bose
    0x05A963, // Bose QuietComfort Ultra Earbuds
    0x05A964, // Bose QuietComfort Ultra Headphones
    0x05A965, // Bose QuietComfort Earbuds II
    0x05A966, // Bose Sport Earbuds
    0xF00209, // Bose NC 700

    // Automotive
    0xCC93A5, // Ford
    0x9DB896, // BMW
    0x7C74F0, // Audi
    0x3E8E2C, // Tesla
    0xF8B635, // Mercedes-Benz
    0x5792F0, // Volkswagen
    0xE49ECA, // Toyota
    0x2D7A23, // Honda

    // Other popular devices
    0xC7A267, // Microsoft Mouse
    0x091300, // Google Chromecast
    0x1E89A7, // Nothing Ear (1)
    0x1E89A8, // Nothing Ear (2)
    0x1E89A9, // Nothing Ear (stick)
    0x0C0B67, // OnePlus Buds Pro
    0x0C0B68, // OnePlus Buds Pro 2
    0x0C0B69, // OnePlus Buds 3
    0x7A754A, // Motorola Edge 40
    0x8520F4, // OPPO Enco X2
    0x8520F5, // OPPO Enco Air3 Pro
    0x62C8E4, // Xiaomi Buds 4 Pro
    0x62C8E5, // Xiaomi Buds 3T Pro
    0x62C8E6, // Xiaomi FlipBuds Pro

    // Trackers / Tags
    0xD446A7, // Pebblebee Tag
    0x2C09D0, // Chipolo ONE Spot
    0xAABBCC, // Tile Mate
    0xAABBCD, // Tile Pro
    0xAABBCE, // Tile Slim
    0x5AFCE4, // SmartTag2

    // Misc
    0xF49F36, // Fitbit Charge 6
    0xF49F37, // Fitbit Versa 4
    0x72EF8D, // Garmin Venu 3
    0xFACE01, // Random TWS 1
    0xFACE02, // Random TWS 2
    0xFACE03, // Random TWS 3
};

constexpr size_t ANDROID_MODEL_IDS_COUNT = sizeof(ANDROID_MODEL_IDS) / sizeof(ANDROID_MODEL_IDS[0]);
