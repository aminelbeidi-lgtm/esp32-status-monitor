#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include "secrets.h"

// ---- Settings ----
const char* WIFI_SSID = "Wokwi-GUEST";   // Wokwi's free simulated Wi-Fi
const char* WIFI_PASS = "";
const char* API_KEY = SECRET_API_KEY;
#define DHT_PIN 15
DHT dht(DHT_PIN, DHT22);

const unsigned long SEND_INTERVAL = 20000;  // 20 s (ThingSpeak free limit is 15 s)
unsigned long lastSend = 0;

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost - reconnecting...");
    connectWiFi();
  }

  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    float temp   = dht.readTemperature();
    long  rssi   = WiFi.RSSI();
    long  uptime = millis() / 1000;

    if (isnan(temp)) {
      Serial.println("ERROR: sensor read failed");
      return;
    }

    String url = "http://api.thingspeak.com/update?api_key=" + String(API_KEY) +
                 "&field1=" + String(temp) +
                 "&field2=" + String(rssi) +
                 "&field3=" + String(uptime);

    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    String response = http.getString();
    http.end();

    Serial.printf("Temp: %.1f C | Signal: %ld dBm | Uptime: %ld s\n", temp, rssi, uptime);
    Serial.printf("HTTP %d, ThingSpeak entry: %s\n", code, response.c_str());
  }
}