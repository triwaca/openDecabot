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

const char *ssid = "ESP_Captive_Portal";
const char *password = "";

DNSServer dnsServer;
const byte DNS_PORT = 53;

AsyncWebServer server(80);

const int LED_PIN = LED_BUILTIN;

void notFound(AsyncWebServerRequest *request) {
  request->redirect("/");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(myIP);

  dnsServer.start(DNS_PORT, "*", myIP);

  server.on("/", HTTP_GET, [myIP](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><title>ESP LED Control</title></head><body>";
    html += "<h1>LED Control on " + myIP.toString() + "</h1>";
    html += "<p><a href=\"/on\">Turn LED ON</a></p>";
    html += "<p><a href=\"/off\">Turn LED OFF</a></p>";
    html += "<p><a href=\"http://" + myIP.toString() + "/\">Open on device browser</a></p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_PIN, LOW);
    request->redirect("/");
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_PIN, HIGH);
    request->redirect("/");
  });

  server.onNotFound(notFound);

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}
