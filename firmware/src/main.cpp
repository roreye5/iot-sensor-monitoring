#include "TempAndHumid.h"
#include "DataSender.h"
#include "LightSensor.h"
#include "PressureSensor.h"
#include <Wire.h>
#include <SPI.h>

#define DHT_TYPE DHT11
#define DHT11PIN 33
#define MHSENSORPIN 36


const char* serverUrl = "https://ece140.frosty-sky-f43d.workers.dev/api/insert";
const char* epUsername = ENTERPRISE_USERNAME;
const char* epPassword = ENTERPRISE_PASSWORD;
const char* ucsdPid = UCSD_PID;
const char* espLocation = ESP_LOCATION;
const char* wifiSsid = WIFI_SSID;
const char* nonEnterpriseWifiPassword = NON_ENTERPRISE_WIFI_PASSWORD;

TempAndHumid tempSensor = TempAndHumid(DHT11PIN, DHT_TYPE);
DataSender senderOfData = DataSender(ucsdPid, espLocation, serverUrl);

void setup() {
  Serial.begin(115200);
  senderOfData.connectToWPAEnterprise(wifiSsid, epUsername, epPassword);
}

void loop() {

    float tempReading = tempSensor.getTemp();
    senderOfData.sendData("temperature", "DHT11", tempReading, "C");
    Serial.println(tempReading);
    delay(10000);
}
