# ATC_MiThermometer

[![GitHub release](https://img.shields.io/github/release/matthias-bs/ATC_MiThermometer?maxAge=3600)](https://github.com/matthias-bs/ATC_MiThermometer/releases)
[![License: MIT](https://img.shields.io/badge/license-MIT-green)](https://github.com/matthias-bs/ATC_MiThermometer/blob/main/LICENSE)

BLE Client Library based on [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) for receiving ATC_MiThermometer Data (as Advertising Data) - currently only the "custom" format is supported, **not** the original atc1441 format!

This project allows to receive data from a battery-powered bluetooth low energy thermometer/hygrometer like the Xiaomi Mijia (LYWSD03MMC) running the custom firmware [ATC_MiThermometer](https://github.com/pvvx/ATC_MiThermometer). The software runs in the Arduino environment on all devices dupported by [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino).

The [ATC_MiThermometer](https://github.com/pvvx/ATC_MiThermometer) firmware sends the sensor and status data as BLE advertisements, i.e. multiple clients can receive and use the sensor data.
