#include "DataSender.h"

DataSender::DataSender(String userId, String location, String serverUrl, int ledPin)
: _userId(userId), _location(location), _serverUrl(serverUrl), _ledPin(ledPin) {
    pinMode(_ledPin, OUTPUT);
    Serial.println("[DataSender] Initialized with user ID, location, and server URL");
}

void DataSender::connectToWiFi(String ssid, String password) {
  Serial.println("[WiFi] Connecting to WiFi...");
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  digitalWrite(_ledPin, HIGH); // Turn on the built-in LED to indicate successful WiFi connection
  Serial.println("\n[WiFi] Connected to WiFi.");
}

void DataSender::connectToWPAEnterprise(String ssid, String username, String password) {
  Serial.println("[WiFi] Connecting to WPA Enterprise...");

  WiFi.disconnect(true);  // Disconnect from any network
  WiFi.mode(WIFI_STA); // Set WiFi to Station Mode

  // Initialize the WPA2 Enterprise parameters
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username.c_str(), username.length());
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username.c_str(), username.length());
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password.c_str(), password.length());
  
  // Enable WPA2 Enterprise
  esp_wifi_sta_wpa2_ent_enable();

  WiFi.begin(ssid.c_str()); // Start connection attempt

  // Wait for connection result
  Serial.print("Waiting for connection...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected to WPA Enterprise successfully!");

  // Optional: Manually set DNS servers if DHCP does not work correctly
  ip_addr_t dnsserver;
  IP_ADDR4(&dnsserver, 8, 8, 8, 8); // Google's DNS as an example
  dns_setserver(0, &dnsserver);
}

bool DataSender::sendData(float temperature, float humidity) {
  Serial.println("[DataSender] Preparing to send data...");
  // Store the last data entry
  _lastData = "{ \"user_id\": " + _userId +
              ", \"location\": \"" + _location +
              "\", \"temperature\": " + String(temperature, 2) +
              ", \"humidity\": " + String(humidity, 2) + " }";
  Serial.println("[DataSender] Data stored.");

  if (_checkRateLimit()) {
    Serial.println("[DataSender] Rate limit check passed, attempting to send data...");
    return _performSend();
  } else {
    Serial.println("[DataSender] Rate limit not passed, send skipped.");
    return false; // Data not sent due to rate limit
  }
}

bool DataSender::_checkRateLimit() {
  unsigned long currentTime = millis();
  const unsigned long minSendInterval = 120000; // 2 minutes

  if (currentTime - _lastSendTime < minSendInterval) {
    Serial.println("[DataSender] Rate limit in effect, waiting...");
    return false; // Don't send data, rate limit in effect
  }
  Serial.println("[DataSender] Rate limit passed.");
  _lastSendTime = currentTime; // Update last send time
  return true;
}

bool DataSender::_performSend() {
  // Check if server URL is valid
  if (_serverUrl.length() == 0) {
    Serial.println("[DataSender] Server URL is empty.");
    return false;
  }

  HTTPClient http;
  Serial.println("[DataSender] Initializing HTTPClient...");

  Serial.println("[DataSender] Serialized JSON data to send: " + _lastData);

  http.begin(_serverUrl.c_str()); // Specify request destination
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(_lastData); // Send the request
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("[DataSender] Data sent successfully, server response: " + response);
    http.end(); //Close connection
    digitalWrite(_ledPin, HIGH); // Turn on the built-in LED to indicate successful data send
    return true; // Data sent successfully
  } else {
    Serial.println("[DataSender] Error on sending POST: " + String(httpResponseCode));
    http.end(); //Close connection
    digitalWrite(_ledPin, LOW); // Turn off the built-in LED to indicate failed data send
    return false; // Failed to send data
  }

}
