/*
 * Captive Portal Decabot
 * Daniel Chagas, Cauan Oliveira, Paulo Roberto Gomes
 *
 * Robô cria uma rede wifi com captive portal que exibe botões de controle
 * remoto que controlam os servo motores. 
 */

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Servo.h>
#include <Adafruit_GFX.h>       // tested on 1.11.10
#include <WEMOS_Matrix_GFX.h>   // tested on 1.4.0iu332
#define buzzer D8

MLED matrix(7); // intensidade 7 (máximo)

int lpos = 1; // posição olho esquerdo
int rpos = 6; // posição olho direito
int aleat = 5;

const char *ssid = "MeuDecabot";
const char *password = "";
Servo servo0;
Servo servo6;

DNSServer dnsServer;
const byte DNS_PORT = 53;

AsyncWebServer server(80);

const int LED_PIN = LED_BUILTIN;
bool ledState = false;

void notFound(AsyncWebServerRequest *request) {
  request->redirect("/");
}

void setup() {
  pinMode(buzzer, OUTPUT);
  matrix.setRotation(2);
  servo0.attach(D0);
  servo6.attach(D6);
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED off (ativo em LOW)
  ledState = false;

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  dnsServer.start(DNS_PORT, "*", myIP);

  // Página raiz (cativa)
  server.on("/", HTTP_GET, [myIP](AsyncWebServerRequest *request) {
    String ipStr = myIP.toString();
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Controle do Robô</title>
  <style>
    :root{
      --bg:#1a1a2e;
      --btn:#16213e;
      --border:#0f3460;
      --txt:#fff;
      --accent:#0f3460;
    }
    *{box-sizing:border-box}
    html,body{
      height:100%;
      margin:0;
    }
    body {
      background: var(--bg);
      font-family: Arial, Helvetica, sans-serif;
      color: var(--txt);
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      gap: 18px;
      user-select: none;
      -webkit-user-select: none;
    }
    h1{margin:0 0 4px 0;font-size:20px;opacity:.9}
    .controle {
      display: grid;
      grid-template-columns: 90px 90px 90px;
      grid-template-rows: 90px 90px 90px;
      gap: 14px;
      touch-action: none; /* evita scroll durante o toque */
    }
    button {
      background: var(--btn);
      color: var(--txt);
      border: 2px solid var(--border);
      border-radius: 16px;
      font-size: 22px;
      cursor: pointer;
      transition: transform .08s ease, background .15s ease;
      outline: none;
      -webkit-tap-highlight-color: transparent;
    }
    button:active{
      transform: scale(0.97);
      background: var(--accent);
    }
    /* posição dos botões */
    .frente  { grid-column: 2; grid-row: 1; }
    .esquerda{ grid-column: 1; grid-row: 2; }
    .direita { grid-column: 3; grid-row: 2; }
    .tras    { grid-column: 2; grid-row: 3; }

    .status{
      font-size:14px;
      opacity:.85;
    }

    @media (max-width: 420px){
      .controle{
        grid-template-columns: 80px 80px 80px;
        grid-template-rows: 80px 80px 80px;
        gap: 12px;
      }
    }
  </style>
</head>
<body>
  <h1>Controle do Robô</h1>

  <div class="controle">
    <button class="frente"   aria-label="Frente"   onpointerdown="press('Front')" onpointerup="release()" onpointerleave="release()">▲</button>
    <button class="esquerda" aria-label="Esquerda" onpointerdown="press('Left')"  onpointerup="release()" onpointerleave="release()">◄</button>
    <button class="direita"  aria-label="Direita"  onpointerdown="press('Right')" onpointerup="release()" onpointerleave="release()">►</button>
    <button class="tras"     aria-label="Trás"     onpointerdown="press('Back')"  onpointerup="release()" onpointerleave="release()">▼</button>
  </div>

  <div class="status">LED status: <strong id="ledState">unknown</strong></div>
  <a href="http://)rawliteral" + ipStr + R"rawliteral(/" style="color:#9bd">Abrir no navegador</a>

  <script>
    let isPressed = false;

    function setLED(state) {
      return fetch('/led?state=' + state)
        .then(r => r.text())
        .then(text => {
          const el = document.getElementById('ledState');
          if (el) el.textContent = text;
        })
        .catch(() => {});
    }

    function press(direction){
      if (isPressed) return;
      isPressed = true;
      setLED(direction);
    }

    function release(){
      if (!isPressed) return;
      isPressed = false;
      setLED('Stop'); // volta para neutro ao SOLTAR
    }

    function getLEDStatus() {
      fetch('/status')
        .then(r => r.text())
        .then(text => {
          const el = document.getElementById('ledState');
          if (el) el.textContent = text;
        })
        .catch(() => {});
    }

    // Se sair da aba, soltaqqeqeqeqqeqqe
    window.addEventListener('blur', release);
    // Estado inicial
    window.onload = getLEDStatus;
  </script>
</body>
</html>
    )rawliteral";

    request->send(200, "text/html", html);
  });

  // Comandos: Front / Left / Right / Back / Stop
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
      String state = request->getParam("state")->value();

      if (state == "Front") {
        digitalWrite(LED_PIN, LOW);
        ledState = true;
        servo0.write(30);
        servo6.write(150);

      } else if (state == "Left") {
        digitalWrite(LED_PIN, LOW);
        ledState = true;
        servo0.write(30);
        servo6.write(90);

      } else if (state == "Right") {
        digitalWrite(LED_PIN, LOW);
        ledState = true;
        servo0.write(90);
        servo6.write(150);

      } else if (state == "Back") {
        digitalWrite(LED_PIN, LOW);
        ledState = true;
        servo0.write(150);
        servo6.write(30);

      } else if (state == "Stop") {
        digitalWrite(LED_PIN, HIGH);
        ledState = false;
        servo0.write(90);
        servo6.write(90);

      } else {
        request->send(400, "text/plain", "Invalid state");
        return;
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

  // animação dos olhos + buzzer (mantida)
  matrix.clear();
  matrix.drawLine(lpos, 2, lpos, 7, LED_ON);
  matrix.drawLine(rpos, 2, rpos, 7, LED_ON);
  matrix.writeDisplay();
  delay(1000);

  // efeito de piscar
  for (int j = 0; j < random(3); j++) {
    for (int i = 2; i < 7; i++) {
      matrix.clear();
      matrix.drawLine(lpos, i, lpos, 7, LED_ON);
      matrix.drawLine(rpos, i, rpos, 7, LED_ON);
      matrix.writeDisplay();
      digitalWrite(buzzer, HIGH);
      delay(15);
      digitalWrite(buzzer, LOW);
      delay(15);
    }
    for (int i = 7; i > 2; i--) {
      matrix.clear();
      matrix.drawLine(lpos, i, lpos, 7, LED_ON);
      matrix.drawLine(rpos, i, rpos, 7, LED_ON);
      matrix.writeDisplay();
      digitalWrite(buzzer, HIGH);
      delay(15);
      digitalWrite(buzzer, LOW);
      delay(15);
    }
  }

  // olhar para os lados
  aleat = random(10);
  if (aleat < 2) {
    lpos = 0;
    rpos = 5;
  } else if (aleat > 7) {
    lpos = 2;
    rpos = 7;
    delay(15);
  } else {
    lpos = 1;
    rpos = 6;
    delay(15);
  }
}