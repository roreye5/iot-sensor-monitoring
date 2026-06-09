#include "TempAndHumid.h"
#include "DataSender.h"
#include "LightSensor.h"
#include "PressureSensor.h"
#include <Wire.h>
#include <SPI.h>
#include "Display.h"



#define DHT_TYPE DHT22
#define DHT22PIN 5
#define MHSENSORPIN 34
#define BUILT_IN_LED_PIN 2


const char* serverUrl = "http://192.168.0.19:8000/api/sensor-data";
const char* epUsername = ENTERPRISE_USERNAME;
const char* epPassword = ENTERPRISE_PASSWORD;
const char* userID = USER_ID;
const char* espLocation = ESP_LOCATION;
const char* wifiSsid = WIFI_SSID;
const char* nonEnterpriseWifiPassword = NON_ENTERPRISE_WIFI_PASSWORD;


TempAndHumid tempSensor = TempAndHumid(DHT22PIN, DHT_TYPE);
// LightSensor lightSensor = LightSensor(MHSENSORPIN);
DataSender senderOfData = DataSender(userID, espLocation, serverUrl, BUILT_IN_LED_PIN);
Display display = Display();

unsigned long lastLightSendTime = 0;
unsigned long lastDisplayUpdateTime = 0;


void setup() {
  display.begin();
  // pinMode(BUILT_IN_LED_PIN, OUTPUT);
  Serial.begin(115200);
  display.showMessage("Powering up...");
  delay(10000); // Wait for 10 seconds before attempting to connect to WiFi
  display.showMessage("WiFi Connecting");
  senderOfData.connectToWiFi(wifiSsid, nonEnterpriseWifiPassword);
  display.showMessage("WiFi connected!");
  // digitalWrite(BUILT_IN_LED_PIN, HIGH); // Turn on the built-in LED to indicate successful WiFi connection
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastDisplayUpdateTime >= 10000) { // Update display every 10 seconds
    float temperature = tempSensor.getTemp();
    float humidity = tempSensor.getHumidity();
    display.showSensorData(temperature, humidity);
    lastDisplayUpdateTime = currentTime; // Update the last display update time
  }



  // Send light data every 2 minutes
  if (currentTime - lastLightSendTime >= 120000) { // 120 seconds = 2 minutes
    float temperature = tempSensor.getTemp();
    float humidity = tempSensor.getHumidity();

    Serial.println("Temperature: " + String(temperature) + " °F");
    Serial.println("Humidity: " + String(humidity) + " %");
    senderOfData.sendData(temperature, humidity);

    lastLightSendTime = currentTime; // Update the last send time
  }


}
