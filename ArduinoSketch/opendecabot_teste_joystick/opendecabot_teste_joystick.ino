#include <esp_now.h>
#include <WiFi.h>

// pinos do joystick
const int xPin = 32;
const int yPin = 33;

// pinos dos botoes
const int swPin = 4;
const int sw1Pin = 13;

// MAC do ESP8266 receptor (coloque o endereço correto)
uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // esse endereço da broadcast para todos os esps

// Estrutura compatível (tem que ser igual a do decabot)
typedef struct {
  bool esquerda;
  bool direita;
  bool cima;
  bool baixo;
  bool centro;
  bool botaoAbre;
  bool botaoFecha;
} JoystickState;

JoystickState js;

// Função para suavizar leitura do joystick
int suavizar(int pin, int samples = 10) {
  long soma = 0;
  for (int i = 0; i < samples; i++) {
    soma += analogRead(pin);
    delayMicroseconds(100);
  }
  return soma / samples;
}

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);

  //-- inicializa o esp-NOW --
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Falha ao adicionar peer");
    return;
  }
  //-- termina a inicialização o esp-NOW --

  //define como os pinos serao usados
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT_PULLUP);
  pinMode(swPin, INPUT_PULLUP);
  pinMode(sw1Pin, INPUT_PULLUP);

  Serial.println("Transmissor iniciado");
}

void loop() {

  // le os valores do joystick e suaviliza as leituras
  int x = suavizar(xPin);
  int y = suavizar(yPin);

  // le se os botões foram apertados
  int sw = digitalRead(swPin);
  int sw1 = digitalRead(sw1Pin);

  // Zonas mortas e limiares
  int centroMin = 1800;
  int centroMax = 2100;

  //manda os dados
  js.esquerda = (x < 1600 );
  js.direita  = (x > 2200 );
  js.cima     = (y < 1600 );
  js.baixo    = (y > 2200 );
  js.centro = (x > centroMin && x < centroMax && y > centroMin && y < centroMax);
  s.botaoFecha = (sw == LOW);
  js.botaoAbre  = (sw1 == LOW);
  
  //debugging
  Serial.println(y);
  Serial.println(x);
  Serial.println(sw);
  Serial.println(sw1);


  // Envia via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&js, sizeof(js));

  if (result == ESP_OK) Serial.println("Dados enviados!");
  else Serial.println("Erro no envio!");

  delay(100);
}