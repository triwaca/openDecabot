/*
 * Captive Portal Decabot
 * Daniel Chagas, Nicole Akemi Takeda, Cauan Oliveira, Paulo Roberto Gomes
 *
 * Robô cria uma rede wifi com captive portal que exibe botões de controle
 * remoto que controlam os servo motores ligados às portas D0 (motor direito)
 * e D6 (motor esquerdo). 
 *
 * To use:
 * - Access the Wi-Fi network 'MeuDecabot' with a device such as smartphone,
 *   tablet or computer, and click on the login message. 
 * - You must see a page with a title and buttons who can control the movement
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


#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>        //tested on 1.2.4 from dvarrel https://github.com/dvarrel/ESPAsyncTCP
#include <ESPAsyncWebServer.h>  //tested on 3.1.0 from lacamera https://github.com/lacamera/ESPAsyncWebServer
#include <DNSServer.h>
#include <Servo.h>
#include <Adafruit_GFX.h>       // tested on 1.11.10 by Adafruit
#include <WEMOS_Matrix_GFX.h>   // tested on 1.4.0 from Thomas O Fredericks https://github.com/thomasfredericks/wemos_matrix_gfx 
#define buzzer D8
#define battery A0
#include "index_html.h"

/* SETUP DECABOT - Change here parameters! Mude aqui parâmetros do seu robô! */
const char *ssid = "MeuDecabot";
const char *password = "";
bool showBattery = false; //if the robot has battery measurement circuit

MLED matrix(7); // intensidade 7 (máximo)
int lpos = 1; // posição olho esquerdo
int rpos = 6; // posição olho direito
int aleat = 5;
bool awake = false;

//motors
Servo motorDir;
Servo motorEsq;
char caractere = ' ';
String txtSerial = "";
int velocidade = 0; //velocidade geral do robô
int direcao = 0;
long timerDist = 0;
//variáveis de calibração
int motorEsqMin = 33; //calibra com 'a'
int motorEsqZero = 93; //calibra com 'b'
int motorEsqMax = 153; //calibra com 'c'
int motorDirMin = 153; //calibra com 'q'
int motorDirZero = 93; //calibra com 'r'
int motorDirMax = 33; //calibra com 's'

//battery vars
int batValue = 0;
const float minBat = 3.2;
bool charging = false;
int chargingImageStep = 0;
long batTimer = 10000;
bool showingBat = false;
bool batteryAllarm = false;

static const uint8_t PROGMEM miniNumbers[10][5] = {{0x2, 0x5, 0x5, 0x5, 0x2},{0x2, 0x6, 0x2, 0x2, 0x7},{0x6, 0x1, 0x2, 0x4, 0x7},{0x6, 0x1, 0x2, 0x1, 0x6},{0x1, 0x5, 0x7, 0x1, 0x1},{0x7, 0x4, 0x6, 0x1, 0x6},{0x3, 0x4, 0x6, 0x5, 0x2},{0x7, 0x1, 0x2, 0x2, 0x2},{0x2, 0x5, 0x2, 0x5, 0x2},{0x2, 0x5, 0x3, 0x1, 0x6}};

//sounds
const int decabotMusic[4][3]{ { 494, 2, 3 }, { 554, 2, 4 }, { 440, 2, 5 }, { 880, 1, 6 } };
static const uint8_t PROGMEM decabotLogo[4][5] = {{0b11001110,0b10101000,0b10101100,0b10101000,0b11001110},{0b00110010,0b01000101,0b01000111,0b01000101,0b00110101},{0b11000100,0b10101010,0b11001010,0b10101010,0b11000100},{0b01110100,0b00100100,0b00100100,0b00100000,0b00100100}};

DNSServer dnsServer;
const byte DNS_PORT = 53;

AsyncWebServer server(80);

const int LED_PIN = LED_BUILTIN;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.print("Decabot ");
  Serial.println(ssid);
  pinMode(buzzer, OUTPUT);
  matrix.setRotation(2);
  motorDir.attach(D0);
  motorEsq.attach(D6);
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
    awake = true;
    String ipStr = myIP.toString();
    

    request->send(200, "text/html", html);
  });

  // Comandos: Front / Left / Right / Back / Stop
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
      String state = request->getParam("state")->value();

      if (state == "Front") {
        frente(100);
      } else if (state == "Left") {
        esquerda(100);
      } else if (state == "Right") {
        direita(100);
      } else if (state == "Back") {
        tras(50);
      } else if (state == "Stop") {
        pare();
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
  for (int i = 0; i < 4; i++) {
    matrix.clear();
    matrix.drawBitmap(0, 2, decabotLogo[i] , 8, 5, LED_ON);
    matrix.writeDisplay();
    tone(buzzer, decabotMusic[i][0], decabotMusic[i][1] * 200);
    delay(decabotMusic[i][1] * 200);
    noTone(buzzer);
  }
  delay(500);
  matrix.clear();
}

void loop() {
  dnsServer.processNextRequest();
  if(Serial.available()>0){
    caractere = Serial.read();
    if(caractere == '\n'){
    	maquinaDeEstados(txtSerial);
        txtSerial = "";
    } else {
     	txtSerial = txtSerial + caractere;
    }
  }
  if(millis()>batTimer){
    if(showingBat){
      batTimer = millis() + 10000;
      showingBat = false;
      if(awake){
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
      } else {
        matrix.clear();
        matrix.drawLine(0, 7, 2, 7, LED_ON);
        matrix.drawLine(5, 7, 7, 7, LED_ON);
        matrix.writeDisplay();
      }
    } else {
      if(showBattery) {
        decabotShowBatLevel();
        batTimer = millis() + 1000;
      }
      showingBat = true;
    }
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->redirect("/");
}

void maquinaDeEstados(String txtDigitado){
  //função que recebe o texto e separa o comando do parâmetro
  String comando = txtDigitado.substring(0,1);
  String parametro = txtDigitado.substring(1,txtDigitado.length());
  Serial.println("Comando " + comando + " e Parametro " + parametro);
  //agora vamos fazer os comandos atuarem no robô!
  switch(comando.charAt(0)){
    case 'a':
    	Serial.println("Calibrando motor Esq Min");
    	motorEsqMin = parametro.toInt();
    	break;
    case 'b':
    	Serial.println("Calibrando motor Esq Zero");
    	motorEsqZero = parametro.toInt();
    	break;
    case 'c':
    	Serial.println("Calibrando motor Esq Max");
    	motorEsqMax = parametro.toInt();
    	break;
    case 'q':
    	Serial.println("Calibrando motor Dir Min");
    	motorDirMin = parametro.toInt();
    	break;
    case 'r':
    	Serial.println("Calibrando motor Dir Zero");
    	motorDirZero = parametro.toInt();
    	break;
    case 's':
    	Serial.println("Calibrando motor Dir Max");
    	motorDirMax = parametro.toInt();
    	break;
    case 'y':
    	//recebe o parametro Y do joystick (cima e baixo)
    	velocidade = parametro.toInt();
    	break;
    case 'x':
    	//recebe o parametro X do joystick (esquerda e direita)
    	direcao = parametro.toInt();
    	moveDir(velocidade - (direcao/2));
    	moveEsq(velocidade + (direcao/2));
    	Serial.println("Virando a " + (String)direcao + " na velocidade" + (String)velocidade);
    	break;
    case 'f':
    	frente(parametro.toInt());
    	break;
    case 't':
    	tras(parametro.toInt());
    	break;
    case 'd':
    	direita(parametro.toInt());
    	break;
    case 'e':
    	esquerda(parametro.toInt());
    	break;
    case 'p':
    	pare();
    	break;
    case '?':
      Serial.print("Decabot ");
      Serial.println(ssid);
      break;
    default:
    	Serial.println("Comando desconhecido!");
  }  
}

void frente(int potencia){
  moveDir(potencia);
  moveEsq(potencia);
  Serial.println("Andando para frente na potencia " + (String)potencia);
}

void tras(int potencia){
  moveDir(-potencia);
  moveEsq(-potencia);
  Serial.println("Andando para tras na potencia " + (String)potencia);
}

void direita(int potencia){
  moveDir(-potencia);
  moveEsq(potencia);
  Serial.println("Virando para direita na potencia " + (String)potencia);
}

void esquerda(int potencia){
  moveDir(potencia);
  moveEsq(-potencia);
  Serial.println("Virando para esquerda na potencia " + (String)potencia);
}

void pare(){
  moveDir(0);
  moveEsq(0);
  Serial.println("Parada!");
}

void moveDir(int potencia){
  int saida = 0;
  if(potencia>=0){
  	saida = map(potencia,0,100,motorDirZero,motorDirMax);
  } else {
    saida = map(potencia,-100,0,motorDirMin,motorDirZero);
  }
  motorDir.write(saida);
}

void moveEsq(int potencia){
  int saida = 0;
  if(potencia>=0){
  	saida = map(potencia,0,100,motorEsqZero,motorEsqMax);
  } else {
    saida = map(potencia,-100,0,motorEsqMin,motorEsqZero);
  }
  motorEsq.write(saida);
}

int readBatteryLevel(){
  int reading = map(analogRead(battery),704,955,0,100);
  if (reading<0) reading = 0;
  if (reading>100) reading = 100;
  if (reading<10){
    if(batteryAllarm){
      batteryAllarm = false;
      noTone(buzzer);
    } else {
      batteryAllarm = true;
      tone(buzzer, 494, 200);
    }
  }
  if(reading==100){
    charging = true;
  } else {
    charging = false;
  }
  return reading;
}

void decabotShowBatLevel(){
  matrix.clear();
  matrix.drawRect(0,2, 7,4, LED_ON);
  matrix.drawLine(7,3,7,4, LED_ON);
  if(charging){
    matrix.fillRect(1,3, chargingImageStep,2, LED_ON);
    chargingImageStep++;
    if(chargingImageStep>5) chargingImageStep = 0;
    matrix.writeDisplay();
    delay(200);
  } else {
    matrix.fillRect(1,3, map(readBatteryLevel(),0,100,0,5),2, LED_ON);
    matrix.writeDisplay();
  }
}

void showMiniNumbers(int value){
  int tens = value/10;
  int units = value - tens*10;
  matrix.clear();
  matrix.drawBitmap(0, 1, miniNumbers[units] , 8, 5, LED_ON);
  matrix.drawBitmap(-4, 1, miniNumbers[tens] , 8, 5, LED_ON);
  matrix.writeDisplay();
}