# ATC_MiThermometer

[![CI](https://github.com/matthias-bs/ATC_MiThermometer/actions/workflows/CI.yml/badge.svg)](https://github.com/matthias-bs/ATC_MiThermometer/actions/workflows/CI.yml)
[![GitHub release](https://img.shields.io/github/release/matthias-bs/ATC_MiThermometer?maxAge=3600)](https://github.com/matthias-bs/ATC_MiThermometer/releases)
[![License: MIT](https://img.shields.io/badge/license-MIT-green)](https://github.com/matthias-bs/ATC_MiThermometer/blob/main/LICENSE)

Arduino BLE Client Library based on [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) for receiving ATC_MiThermometer Data (as Advertising Data) - both the "custom" format and the original "atc1441" format are supported.

This project allows to receive data from a battery-powered bluetooth low energy thermometer/hygrometer like the Xiaomi Mijia (LYWSD03MMC) running the custom firmware [ATC_MiThermometer](https://github.com/pvvx/ATC_MiThermometer). The software runs in the Arduino environment on all devices dupported by [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino).

The [ATC_MiThermometer](https://github.com/pvvx/ATC_MiThermometer) firmware sends the sensor and status data as BLE advertisements, i.e. multiple clients can receive and use the sensor data.

This project is the successor of [ESP32_ATC_MiThermometer_Library](https://github.com/matthias-bs/ESP32_ATC_MiThermometer_Library) - with all its benefits inherited from [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino).

## Example
```
#include "ATC_MiThermometer.h"

const int scanTime = 5; // BLE scan time in seconds

// List of known sensors' BLE addresses
std::vector<std::string> knownBLEAddresses = {"a4:c1:38:b8:1f:7f", "a4:c1:38:bf:e1:bc"};

ATC_MiThermometer miThermometer(knownBLEAddresses);


void setup() {
    Serial.begin(115200);
    
    // Initialization
    miThermometer.begin();
}

void loop() {
    // Set sensor data invalid
    miThermometer.resetData();
    
    // Get sensor data - run BLE scan for <scanTime>
    unsigned found = miThermometer.getData(scanTime);

    for (int i=0; i < miThermometer.data.size(); i++) {  
        if (miThermometer.data[i].valid) {
            Serial.println();
            Serial.printf("Sensor %d: %s\n", i, knownBLEAddresses[i].c_str());
            Serial.printf("%.2f°C\n", miThermometer.data[i].temperature/100.0);
            Serial.printf("%.2f%%\n", miThermometer.data[i].humidity/100.0);
            Serial.printf("%.3fV\n",  miThermometer.data[i].batt_voltage/1000.0);
            Serial.printf("%d%%\n",   miThermometer.data[i].batt_level);
            Serial.printf("%ddBm\n",  miThermometer.data[i].rssi);
            Serial.println();
         }
    }
    Serial.print("Devices found: ");
    Serial.println(found);
    Serial.println();

    // Delete results from BLEScan buffer to release memory
    miThermometer.clearScanResults();
    delay(5000);
}
```
## Source Code Documentation
https://matthias-bs.github.io/ATC_MiThermometer/

## Alternative
You might want to have a look at [Theengs Decoder](https://decoder.theengs.io/). It is also built on NimBLE-Arduino, runs on ESP32, seems to decode both variants of the alternative LYWSD03MMC firmware and many devices more.
