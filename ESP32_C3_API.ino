#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>

// ====== LoRa Pins ======
#define ss 5
#define rst 14
#define dio0 2

// ====== WiFi Credentials ======
const char* ssid = "Rupsa";
const char* password = "ljkm5467";

// ====== Web Server on port 80 ======
WebServer server(80);

// ====== Global variable to store latest LoRa data ======
String latestLoRaData = "";
int latestRSSI = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // ---- WiFi Setup ----
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ---- LoRa Setup ----
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed...");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa initialized!");

  // ---- API Endpoints ----
  server.on("/getData", HTTP_GET, []() {
    if (latestLoRaData != "") {
      String response = "{ \"data\": \"" + latestLoRaData + "\", \"rssi\": " + String(latestRSSI) + " }";
      server.send(200, "application/json", response);
    } else {
      server.send(200, "application/json", "{ \"message\": \"No data received yet\" }");
    }
  });

  server.on("/health", HTTP_GET, []() {
    server.send(200, "application/json", "{ \"status\": \"OK\", \"ip\": \"" + WiFi.localIP().toString() + "\" }");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle API requests
  server.handleClient();

  // Handle LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String LoRaData = "";
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }
    latestLoRaData = LoRaData;
    latestRSSI = LoRa.packetRssi();

    Serial.print("Received packet: ");
    Serial.print(latestLoRaData);
    Serial.print(" with RSSI ");
    Serial.println(latestRSSI);
  }
}
