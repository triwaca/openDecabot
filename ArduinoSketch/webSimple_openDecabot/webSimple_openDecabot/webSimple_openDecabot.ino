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
bool ledState = false;

void notFound(AsyncWebServerRequest *request) {
  request->redirect("/");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED off (active low)
  ledState = false;

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  dnsServer.start(DNS_PORT, "*", myIP);

  // Root page
  server.on("/", HTTP_GET, [myIP](AsyncWebServerRequest *request) {
    String ipStr = myIP.toString();
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP LED Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 2em;
      background: #f0f0f0;
    }
    h1 {
      color: #333;
      font-size: 24px;
    }
    button {
      padding: 15px 30px;
      font-size: 18px;
      margin: 10px;
      border: none;
      border-radius: 5px;
      cursor: pointer;
    }
    #onBtn { background-color: #4CAF50; color: white; }
    #offBtn { background-color: #f44336; color: white; }
    #status {
      margin-top: 20px;
      font-size: 20px;
      color: #333;
    }
    a {
      display: block;
      margin-top: 30px;
      color: #007bff;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h1>ESP8266 LED Control - IP: )rawliteral" + ipStr + R"rawliteral(</h1>
  <button id="onBtn" onclick="setLED('on')">Turn ON</button>
  <button id="offBtn" onclick="setLED('off')">Turn OFF</button>
  <div id="status">LED status: <strong id="ledState">unknown</strong></div>
  <a href="http://)rawliteral" + ipStr + R"rawliteral(/">Open on device browser</a>

  <script>
    function setLED(state) {
      fetch('/led?state=' + state)
        .then(response => response.text())
        .then(data => {
          document.getElementById('ledState').textContent = data;
        });
    }

    function getLEDStatus() {
      fetch('/status')
        .then(response => response.text())
        .then(data => {
          document.getElementById('ledState').textContent = data;
        });
    }

    window.onload = getLEDStatus;
  </script>
</body>
</html>
    )rawliteral";

    request->send(200, "text/html", html);
  });

  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
      String state = request->getParam("state")->value();
      if (state == "on") {
        digitalWrite(LED_PIN, LOW);
        ledState = true;
      } else {
        digitalWrite(LED_PIN, HIGH);
        ledState = false;
      }
      request->send(200, "text/plain", ledState ? "ON" : "OFF");
    } else {
      request->send(400, "text/plain", "Missing parameter");
    }
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", ledState ? "ON" : "OFF");
  });

  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}
