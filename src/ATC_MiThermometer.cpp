///////////////////////////////////////////////////////////////////////////////////////////////////
// ATC_MiThermometer.cpp
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
// 20221223 Added support for ATC1441 format
// 20240403 Added reedSwitchState, gpioTrgOutput, controlParameters,
//          tempTriggerEvent &humiTriggerEvent
// 20240425 Added device name
// 20250125 Updated for NimBLE-Arduino v2.x
//
// ToDo:
// -
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <ATC_MiThermometer.h>

/*!
 * \class ScanCallbacks
 *
 * \brief Callback for advertised device found during scan
 */
class ScanCallbacks : public NimBLEScanCallbacks
{
private:
    void onDiscovered(const NimBLEAdvertisedDevice *advertisedDevice) override
    {
        log_v("Discovered Advertised Device: %s", advertisedDevice->toString().c_str());
    }

    void onResult(BLEAdvertisedDevice *advertisedDevice)
    {
        log_d("Advertised Device: %s", advertisedDevice->toString().c_str());
        /*
         * Here we add the device scanned to the whitelist based on service data but any
         * advertised data can be used for your preffered data.
         */
        if (advertisedDevice->haveServiceData())
        {
            /* If this is a device with data we want to capture, add it to the whitelist */
            if (advertisedDevice->getServiceData(NimBLEUUID("181A")) != "")
            {
                log_d("Adding %s to whitelist", std::string(advertisedDevice->getAddress()).c_str());
                NimBLEDevice::whiteListAdd(advertisedDevice->getAddress());
            }
        }
    }

    void onScanEnd(const NimBLEScanResults &results, int reason) override
    {
        log_v("Scan Ended; reason = %d", reason);
    }
} scanCallbacks;

// Set up BLE scanning
void ATC_MiThermometer::begin(bool activeScan)
{
    NimBLEDevice::init("ble-scan");
    _pBLEScan = NimBLEDevice::getScan();  // create new scan
    _pBLEScan->setScanCallbacks(&scanCallbacks);
    _pBLEScan->setActiveScan(activeScan);
    _pBLEScan->setFilterPolicy(BLE_HCI_SCAN_FILT_NO_WL);
    _pBLEScan->setInterval(100);
    _pBLEScan->setWindow(99);

}

// Get sensor data by running BLE device scan
unsigned ATC_MiThermometer::getData(uint32_t scanTime)
{
    // Start scanning
    // Blocks until all known devices are found or scanTime is expired
    BLEScanResults foundDevices = _pBLEScan->getResults(scanTime * 1000, false);

    log_d("Whitelist contains:");
    for (auto i = 0; i < NimBLEDevice::getWhiteListCount(); ++i)
    {
        log_d("%s", NimBLEDevice::getWhiteListAddress(i).toString().c_str());
    }

    log_d("Assigning scan results...");
    for (unsigned i = 0; i < foundDevices.getCount(); i++)
    {
        log_d("haveName(): %d", foundDevices.getDevice(i)->haveName());
        log_d("getName(): %s", foundDevices.getDevice(i)->getName().c_str());

        // Match all devices found against list of known sensors
        for (unsigned n = 0; n < _known_sensors.size(); n++)
        {
            log_d("Found: %s  comparing to: %s",
                  foundDevices.getDevice(i)->getAddress().toString().c_str(),
                  _known_sensors[n].c_str());
            if (foundDevices.getDevice(i)->getAddress().toString() == _known_sensors[n])
            {
                log_d(" -> Match! Index: %d", n);
                data[n].valid = true;

                int len = foundDevices.getDevice(i)->getServiceData().length();
                log_d("Length of ServiceData: %d", len);

                data[n].name = foundDevices.getDevice(i)->getName();
                if (len == 15)
                {
                    log_d("Custom format");
                    // Temperature
                    int temp_msb = foundDevices.getDevice(i)->getServiceData().c_str()[7];
                    int temp_lsb = foundDevices.getDevice(i)->getServiceData().c_str()[6];
                    data[n].temperature = (temp_msb << 8) | temp_lsb;

                    // Humidity
                    int hum_msb = foundDevices.getDevice(i)->getServiceData().c_str()[9];
                    int hum_lsb = foundDevices.getDevice(i)->getServiceData().c_str()[8];
                    data[n].humidity = (hum_msb << 8) | hum_lsb;

                    // Battery voltage
                    int volt_msb = foundDevices.getDevice(i)->getServiceData().c_str()[11];
                    int volt_lsb = foundDevices.getDevice(i)->getServiceData().c_str()[10];
                    data[n].batt_voltage = (volt_msb << 8) | volt_lsb;

                    // Battery state [%]
                    data[n].batt_level = foundDevices.getDevice(i)->getServiceData().c_str()[12];

                    // Count
                    data[n].count = foundDevices.getDevice(i)->getServiceData().c_str()[13];

                    // Flags
                    uint8_t flagsByte = foundDevices.getDevice(i)->getServiceData().c_str()[14];
                    data[n].reedSwitchState = flagsByte & 0x01;          // Extract bit 0 (Reed Switch)
                    data[n].gpioTrgOutput = (flagsByte >> 1) & 0x01;     // Extract bit 1 (GPIO_TRG pin output)
                    data[n].controlParameters = (flagsByte >> 2) & 0x01; // Extract bit 2 (Control parameters)
                    data[n].tempTriggerEvent = (flagsByte >> 3) & 0x01;  // Extract bit 3 (Temperature trigger event)
                    data[n].humiTriggerEvent = (flagsByte >> 4) & 0x01;  // Extract bit 4 (Humidity trigger event)
                }
                else if (len == 13)
                {
                    log_d("ATC1441 format");

                    // Temperature
                    int temp_lsb = foundDevices.getDevice(i)->getServiceData().c_str()[7];
                    int temp_msb = foundDevices.getDevice(i)->getServiceData().c_str()[6];
                    data[n].temperature = (temp_msb << 8) | temp_lsb;
                    data[n].temperature *= 10;

                    // Humidity
                    data[n].humidity = foundDevices.getDevice(i)->getServiceData().c_str()[8];
                    data[n].humidity *= 100;

                    // Battery voltage
                    int volt_lsb = foundDevices.getDevice(i)->getServiceData().c_str()[11];
                    int volt_msb = foundDevices.getDevice(i)->getServiceData().c_str()[10];
                    data[n].batt_voltage = (volt_msb << 8) | volt_lsb;

                    // Battery state [%]
                    data[n].batt_level = foundDevices.getDevice(i)->getServiceData().c_str()[9];
                }
                else
                {
                    log_d("Unknown ServiceData format");
                }

                // Received Signal Strength Indicator [dBm]
                data[n].rssi = foundDevices.getDevice(i)->getRSSI();
            }
            else
            {
                log_d();
            }
        }
    }
    return foundDevices.getCount();
}

// Set all array members invalid
void ATC_MiThermometer::resetData(void)
{
    for (int i = 0; i < _known_sensors.size(); i++)
    {
        data[i].valid = false;
    }
}
