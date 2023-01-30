// non-blocking Mijia lywsd03mmc thermometers acquisition
// Jean-Paul PETILLON (https://github.com/Art-ut-Kia)
//
// based on this library:
// https://github.com/matthias-bs/ATC_MiThermometer
// Mijia lywsd03mmc thermometers are first to be loaded with a custom firmware as per:
// https://pvvx.github.io/ATC_MiThermometer/TelinkMiFlasher.html

#include "ATC_MiThermometer.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

// List of known sensors' BLE addresses
std::vector<std::string> knownBLEAddresses = {"a4:c1:38:02:8B:E0", "a4:c1:38:64:8F:10", "a4:c1:38:F3:C1:B4"};
ATC_MiThermometer miTh(knownBLEAddresses);

MiThData_S miThData[3];
SemaphoreHandle_t mutex[3];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  for (int i=0; i<3; i++) mutex[i] = xSemaphoreCreateMutex();
  miTh.begin(); // initialization of Mijia sensors acquisition
  xTaskCreate(miThReadingTask,   "miThTask",      10000,  NULL,        1,   NULL); // stack size: tried 1000, not sufficient
  //            task function, name of task, stack size, param, priority, handle
}

void loop() {
  int count = 0;
  for (int i=0; i < 3; i++) {

    // gets a local copy of the ith sensor reading
    // mutex protection ensures reading struct integrity
    MiThData_S miThDat;
    if (xSemaphoreTake(mutex[i], (TickType_t)10)==pdTRUE) {
      miThDat = miThData[i];
      xSemaphoreGive(mutex[i]);
    } else miThDat.valid = false;

    // displays sensor # and sensor reading if valid
    if (miThDat.valid) {
      count++;
      Serial.printf("sensor #%d -> ", i);
      Serial.printf("temperature: %.2fC ; ",   miThDat.temperature *0.01f );
      Serial.printf("humidity: %.2f%% ; ",     miThDat.humidity    *0.01f );
      Serial.printf("voltage: %.3fV ; ",       miThDat.batt_voltage*0.001f);
      Serial.printf("battery charge: %d%% ; ", miThDat.batt_level);
      Serial.printf("RF power: %ddBm\n",       miThDat.rssi);
    }
  }
  if (count) Serial.println();
  vTaskDelay(6000 / portTICK_PERIOD_MS);
}

void miThReadingTask(void *pvParameters) {
  while (1) {
    miTh.resetData(); // Set sensor data invalid
    miTh.getData(5); // get sensor data (run BLE scan for 5 seconds)
    // makes a copy of each sensor reading under mutex protection
    for (int i=0; i < 3; i++) if (xSemaphoreTake(mutex[i], (TickType_t)10)==pdTRUE) {
      miThData[i] = miTh.data[i];
      xSemaphoreGive(mutex[i]);
    } else miThData[i].valid = false;
    miTh.clearScanResults(); // clear results from BLEScan buffer to release memory
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
