/*
 * Captive Portal Decabot
 * Daniel Chagas
 * 
 * Creates a Captive Portal webpage with simple comands. 
 * 
 */

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

// Wi-Fi Access Point credentials
const char *ssid = "ESP_Captive_Portal";
const char *password = "";

// DNS server to redirect all domains
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Web server running on port 80
AsyncWebServer server(80);

// LED pin (NodeMCU D4 -> GPIO2)
const int LED_PIN = LED_BUILTIN;

void notFound(AsyncWebServerRequest *request) {
  request->redirect("/");
}

void setup() {
  // Initialize Serial and LED pin
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED off by default

  // Start Wi-Fi in AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Setup DNS to redirect all domains to the ESP IP
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // Main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><title>ESP LED Control</title></head><body>";
    html += "<h1>LED Control</h1>";
    html += "<p><a href=\"/on\">Turn LED ON</a></p>";
    html += "<p><a href=\"/off\">Turn LED OFF</a></p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Turn LED on
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_PIN, LOW); // Turn LED on (active low)
    request->redirect("/");
  });

  // Turn LED off
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_PIN, HIGH); // Turn LED off
    request->redirect("/");
  });

  // Catch-all for unknown routes (captive behavior)
  server.onNotFound(notFound);

  // Start web server
  server.begin();
}

void loop() {
  dnsServer.processNextRequest(); // Keep DNS server responsive
}
