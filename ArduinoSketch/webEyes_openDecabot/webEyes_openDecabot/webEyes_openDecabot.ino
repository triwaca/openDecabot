/*
 * Captive Portal Decabot
 * Daniel Chagas
 * 
 * Creates a Captive Portal webpage with simple comands. 
 * 
 * To use:
 * - Access the Wi-Fi network 'MeuDecabot' with a device such as smartphone,
 *   tablet or computer, and click on the login message. 
 * - You must see a page with a title and two buttons who can control the 
 *   blue LED inside your Decabot (must be visible from the top or from
 *   the frontal holes. 
 * - Click on the buttons to change it's state.
 * 
 * Hardware:
 * Wemos D1 Mini - https://www.wemos.cc/en/latest/d1/index.html
 * Matrix Led Shield - https://github.com/wemos/WEMOS_Matrix_LED_Shield_Arduino_Library
 * Decabot Mini Shield
 * 
 * PORTS                             _____________
 *                                  /             \
 *                             RST |*  ---------  *| TX 
 *           battery ---------- A0 |* |         | *| RX
 *         SERVO esq --GPIO16-- D0 |* |  ESP    | *| D1 ---I2C--- SCL
 *    led matrix CLK ---------- D5 |* |  8266   | *| D2 ---I2C--- SDA
 *         SERVO dir --GPIO12 - D6 |* |         | *| D3 --GPIO0-- SERVO
 *    led matrix DIN ---------- D7 |* |         | *| D4 --GPIO2
 *            buzzer --GPIO15-- D8 |*  ---------  *| GND
 *                             3V3 |*             *| 5V
 *                                  \              |
 *                            reset  |      D1mini |
 *                                    \___________/
 */

#include <Adafruit_GFX.h>       //tested on version 1.11.10
#include <WEMOS_Matrix_GFX.h>   //tested on version 1.4.0
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ArduinoUniqueID.h>

const char *ssid = "MeuDecabot";
const char *password = "";

MLED matrix(7); //set intensity=7 (maximum)

//emotion variables
int eyeTypeVar = 4; //select between 0 to 4
int eyePosLR = 0; //-1, 0 or 1
int eyePosUD = 0; //-1, 0 or 1
int eyeBrowPosUD = -2; //0 close to 5 full open
int eyeBrowEmotion = 0; //-1, 0 or 1
int matrixUpdate = 0;
int blinkTimer = 2000;
int blinkState = 0;
int lastCommandTimmer = 0;

int temp = 0;
int positionLoadLine = 0;

//sounds
const int decabotMusic[4][3]{{494,2,3},{554,2,4},{440,2,5},{880,1,6}};

//bitmaps
static const uint8_t PROGMEM eyes_bmp[7][8] = {
  {B00000000,B00000000,B10100101,B01000010,B10100101,B00000000,B00000000,B00000000},  //dead
  {B00000000,B00000000,B00001000,B00011000,B00001000,B00001000,B00011100,B00000000},  //1 battery levels
  {B00000000,B00000000,B00111000,B00000100,B00011000,B00100000,B00111100,B00000000},  //2
  {B00000000,B00000000,B00111000,B00000100,B00011000,B00000100,B00111000,B00000000},  //3
  {B00000000,B00000000,B00010100,B00100100,B00111100,B00000100,B00000100,B00000000},  //4
  {B00000000,B00000000,B00111100,B00100000,B00111000,B00000100,B00111000,B00000000},  //5
  {B01010000,B01010000,B11111000,B11111000,B01110000,B00100000,B00010000,B00001111}   // charging
};

static const uint8_t PROGMEM numbers[20][8] = {
  {0b00111100,0b01111110,0b01100110,0b01100110,0b01100110,0b01100110,0b01111110,0b00111100}, //0
  {0b00011000,0b00111000,0b01111000,0b00011000,0b00011000,0b00011000,0b01111110,0b01111110}, //1
  {0b00111100,0b01111110,0b01100110,0b00001110,0b00011100,0b00111000,0b01111110,0b01111110}, //2
  {0b01111100,0b01111110,0b00000110,0b00011110,0b00011110,0b00000110,0b01111110,0b01111100}, //3
  {0b00011110,0b00111110,0b00110110,0b01100110,0b01111110,0b01111110,0b00000110,0b00000110}, //4
  {0b01111110,0b01111110,0b01100000,0b01111100,0b01111110,0b00000110,0b01111110,0b01111100}, //5
  {0b00111100,0b01111100,0b01100000,0b01111100,0b01111110,0b01100110,0b01111110,0b00111100}, //6
  {0b01111110,0b01111110,0b00000110,0b00001110,0b00011100,0b00011000,0b00011000,0b00011000}, //7
  {0b00111100,0b01111110,0b01100110,0b00111100,0b01111110,0b01100110,0b01111110,0b00111100}, //8
  {0b00111100,0b01111110,0b01100110,0b01111110,0b00111110,0b00000110,0b00111110,0b00111100}, //9
  {0b11001110,0b11011111,0b11011011,0b11011011,0b11011011,0b11011011,0b11011111,0b11001110}, //10
  {0b01100011,0b11100111,0b11100111,0b01100011,0b01100011,0b01100011,0b01100011,0b01100011}, //11
  {0b11011110,0b11011111,0b11000011,0b11000111,0b11001100,0b11011000,0b11011111,0b11011111}, //12
  {0b11011110,0b11011111,0b11000011,0b11001111,0b11001111,0b11000011,0b11011111,0b11011110}, //13
  {0b11000111,0b11001111,0b11011111,0b11011011,0b11011111,0b11011111,0b11000011,0b11000011}, //14
  {0b11011111,0b11011111,0b11011000,0b11011110,0b11011111,0b11000011,0b11011111,0b11011110}, //15
  {0b11001110,0b11011110,0b11011000,0b11011110,0b11011111,0b11011011,0b11011111,0b11001110}, //16
  {0b11011111,0b11011111,0b11000011,0b11000111,0b11001110,0b11001100,0b11001100,0b11001100}, //17
  {0b11001110,0b11011111,0b11011011,0b11001110,0b11011111,0b11011011,0b11011111,0b11001110}, //18
  {0b11001110,0b11011111,0b11011011,0b11011111,0b11001111,0b11000011,0b11001111,0b11001110}  //19
};

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
  //decabotMatrixNoise();
  Serial.println("");
  Serial.println("");
  decabotMessage("Copyright (C) Decano Robotics - www.decabot.com");
  decabotMessage("Decabot Version: openDecabot");
  //decabotMessage("Robot ID " + decabotUniqueID());
  matrix.setRotation(2);
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
  matrix.clear();
  matrix.drawLine(1,2, 1,7, LED_ON);
  matrix.drawLine(6,2, 6,7, LED_ON);
  matrix.writeDisplay();
}

void loop() {
  dnsServer.processNextRequest();
  /*
  if(millis() > blinkTimer) decabotMatrixBlink();
  if(millis() > lastCommandTimmer + 10000){
    //bored!
    eyeBrowPosUD = 2;
    eyeBrowEmotion = -1;
  }
  decabotMatrixExpressions();
  */
}

void decabotMessage(String text){
  if(text == "OK!"){
    Serial.println(text);
  } else {
    Serial.print(millis());
    Serial.print(" - "); 
    if(text.endsWith("...")){
      Serial.print(text);
    } else {
      Serial.println(text);
    } 
  }
}

String decabotUniqueID(){
  String id = "";
  for (size_t i = 0; i < UniqueIDsize; i++){
    if (UniqueID[i] < 0x10) id = id + "0";
    id = id + UniqueID[i];
  }
  return id;
}

/*
void decabotMatrixExpressions(){
  //exibits faces and colors
  if(millis()>matrixUpdate){
    decabotMatrixEye(eyeTypeVar,eyePosLR,eyePosUD);
    if(eyeBrowPosUD!=5) decabotMatrixEyeBrow(eyeBrowEmotion,eyeBrowPosUD);
    matrix.writeDisplay();
    matrixUpdate = millis() + 250;
  }
}

void decabotMatrixFace(int draw){
  decabotMessage("Change face to " + (String) draw);
  matrix.clear();
  matrix.drawBitmap(0, 0, eyes_bmp[draw], 8, 8, LED_ON);
  matrix.writeDisplay();  
}

void decabotMatrixBitmap(int draw){
  decabotMessage("Change face to " + (String) draw);
  matrix.clear();
  matrix.drawBitmap(0, 0, eyes_bmp[draw], 8, 8, LED_ON);
  matrix.writeDisplay();  
}

void decabotMatrixNumber(int num){
  decabotMessage("Exibit number " + (String) num);
  matrix.clear();
  matrix.drawBitmap(0, 0, numbers[num], 8, 8, LED_ON);
  matrix.writeDisplay();  
}

void decabotMatrixLoadingLine(int high){
  //matrix.clear();
  matrix.drawLine(positionLoadLine-3,high,positionLoadLine,high, LED_ON);
  matrix.drawLine(positionLoadLine-3,high+1,positionLoadLine,high+1, LED_ON);
  matrix.drawLine(positionLoadLine-4,high,positionLoadLine-4,high+1, LED_OFF);
  matrix.writeDisplay();
  positionLoadLine++;
  if(positionLoadLine>11) positionLoadLine = 0;
}

void decabotMatrixDoneLoadingLine(int high){
  matrix.drawLine(0,high,7,high, LED_ON);
  matrix.drawLine(0,high+1,7,high+1, LED_ON);
  matrix.writeDisplay();
}

void decabotMatrixEye(int eyeType,int eyeLrPosition, int eyeUdPosition){
  matrix.clear();
  int lx = 1+eyeLrPosition;
  int rx = 6+eyeLrPosition;
  int ly = 5-eyeUdPosition;
  int ry = 5-eyeUdPosition;
  if(eyeType==0){
    matrix.drawLine(lx,ly,lx,ly-3, LED_ON);
    matrix.drawLine(rx,ry,rx,ry-3, LED_ON);
  }
  if(eyeType==1){
    matrix.drawLine(lx,ly+1,lx,ly-1, LED_ON);
    matrix.drawLine(lx+1,ly+1,lx+1,ly-1, LED_ON);
    
    matrix.drawLine(rx,ry+1,rx,ry-1, LED_ON);
    matrix.drawLine(rx-1,ry+1,rx-1,ry-1, LED_ON);
  }
  if(eyeType==2){
    matrix.drawLine(1,6,1,3, LED_ON);
    matrix.drawLine(2,6,2,3, LED_ON);
    matrix.drawLine(lx+1,6,lx+1,3, LED_ON);
    matrix.drawPixel(lx+1,ly, LED_OFF);
    matrix.drawLine(lx-1,6,lx-1,3, LED_OFF);
    
    matrix.drawLine(5,6,5,3, LED_ON);
    matrix.drawLine(6,6,6,3, LED_ON);
    matrix.drawLine(rx-1,6,rx-1,3, LED_ON);
    matrix.drawPixel(rx-1,ry, LED_OFF);
    matrix.drawLine(rx+1,6,rx+1,3, LED_OFF);
  }
  if(eyeType==3){
    matrix.drawLine(lx,ly+1,lx,ly, LED_ON);
    matrix.drawLine(lx+1,ly+1,lx+1,ly, LED_ON);
    if(lx<1){
      matrix.drawPixel(lx+2,ly-1, LED_ON);
    } else {
      matrix.drawPixel(lx-1,ly-1, LED_ON);
    }
    
    
    matrix.drawLine(rx,ry+1,rx,ry, LED_ON);
    matrix.drawLine(rx-1,ry+1,rx-1,ry, LED_ON);
    if(rx>6){
      matrix.drawPixel(rx-2,ry-1, LED_ON);
    } else {
      matrix.drawPixel(rx+1,ry-1, LED_ON);
    }
  }
  if(eyeType==4){
    matrix.drawPixel(lx,ly, LED_ON);
    matrix.drawPixel(rx,ry, LED_ON);
  }
}

void decabotMatrixEyeBrow(int emotion,int high){
  decabotEyeBrowElement(0,5-(high*1)-emotion);
  decabotEyeBrowElement(1,5-(high*1));
  decabotEyeBrowElement(2,5-(high*1)+emotion);
  matrix.drawLine(3,0,3,5-(high*1)+emotion, LED_OFF);
  matrix.drawLine(4,0,4,5-(high*1)+emotion, LED_OFF);
  decabotEyeBrowElement(5,5-(high*1)+emotion);
  decabotEyeBrowElement(6,5-(high*1));
  decabotEyeBrowElement(7,5-(high*1)-emotion);
}

void decabotMatrixEyeBrowElement(int colum,int heigh){
  matrix.drawLine(colum,0,colum,heigh, LED_OFF);
  matrix.drawPixel(colum,heigh, LED_ON);
}

void decabotMatrixNoise(){
  for(int i=0;i<4;i++){
    matrix.drawPixel(random(8),random(8), LED_ON);
  }
  for(int i=0;i<4;i++){
    matrix.drawPixel(random(8),random(8), LED_OFF);
  }
  matrix.writeDisplay();
}

void decabotMatrixBlink(){
  blinkState++;
  if(eyeTypeVar < 3){
    //long blink
    if(blinkState<9){
      blinkTimer = millis() + 50;
    } else {
      blinkTimer = millis() + (random(10)*1000);
      blinkState = 0;
    }
    if(blinkState<6){
      eyeBrowPosUD = 5 - blinkState - 2;
    } else {
      eyeBrowPosUD = blinkState - 7;
    }
  } else {
    //short blink
    if(blinkState<5){
      blinkTimer = millis() + 50;
    } else {
      blinkTimer = millis() + (random(10)*1000);
      blinkState = 0;
    }
    if(blinkState<3){
      eyeBrowPosUD = 5 - blinkState - 3;
    } else {
      eyeBrowPosUD = blinkState - 7;
    }
  }
  if(connectedToServer){
    decabotEye(eyeTypeVar,eyePosLR,eyePosUD);
    if(eyeBrowPosUD!=5) decabotEyeBrow(eyeBrowEmotion,eyeBrowPosUD);
    matrix.writeDisplay();
  } else {
    decabotMatrixNoise();
  }
  matrixUpdate = millis() + 250;
  decabotMessage("Blink " + (String) blinkState + "!");
}

void decabotBuzzerThemeMusic(){
  decabotMessage("Playing Decabot Music Theme...");
  eyePosLR = 0;
  eyePosUD = 0;
  eyeBrowEmotion = 0;
  for(int i=0;i<4;i++){
    eyeBrowPosUD = i+1;
    decabotMatrixExpressions();
    tone(buzzerPin, decabotMusic[i][0], decabotMusic[i][1]*200);
    delay(decabotMusic[i][1]*200);
    noTone(buzzerPin);
  }
  decabotMessage("OK!");
}
*/
