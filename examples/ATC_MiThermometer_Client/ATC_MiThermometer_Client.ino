///////////////////////////////////////////////////////////////////////////////////////////////////
// ATC_MiThermometer_Client.cpp
//
// Bluetooth low energy thermometer/hygrometer sensor client for MCUs supported by NimBLE-Arduino.
// For sensors running ATC_MiThermometer firmware (see https://github.com/pvvx/ATC_MiThermometer)
//
// https://github.com/matthias-bs/ATC_MiThermometer
//
// Based on:
// ---------
// NimBLE-Arduino by h2zero (https://github.com/h2zero/NimBLE-Arduino)
// LYWSD03MMC.py by JsBergbau (https://github.com/JsBergbau/MiTemperature2)
//
// created: 11/2022
//
//
// MIT License
//
// Copyright (c) 2022 Matthias Prinke
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// History:
//
// 20221123 Created
//
// To Do: 
// -
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <ATC_MiThermometer.h>

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
