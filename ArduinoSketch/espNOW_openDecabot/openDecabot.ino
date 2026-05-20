#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <WEMOS_Matrix_GFX.h>

#define buzzer D8
#define LED_PIN LED_BUILTIN

int lpos = 1; // posição olho esquerdo
int rpos = 6; // posição olho direito
int aleat = 5;

bool ledState = false;
MLED matrix(5);

// Estrutura recebida (deve ser igual à do transmissor)
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

JoystickState jsState;

// Servos
Servo servoEsq; 
Servo servoDir; 
Servo garra; 


// Posições dos servos
int parado = 90;   
int frenteEsq = 150;    
int frenteDir = 30;  
int trasEsq   = 30;  
int trasDir   = 150;    

// Função para movimentar o robô
void mover(String direcao) {
  if (direcao == "frente") {
    servoEsq.write(frenteEsq);
    servoDir.write(frenteDir);
  } else if (direcao == "tras") {
    servoEsq.write(trasEsq);
    servoDir.write(trasDir);
  } else if (direcao == "esquerda") {
    servoEsq.write(frenteEsq);
    servoDir.write(trasDir);
  } else if (direcao == "direita") {
    servoEsq.write(trasEsq);
    servoDir.write(frenteDir);
  } else if (direcao == "cima_esquerda") {
    servoEsq.write(frenteEsq);
    servoDir.write(50);
  } else if (direcao == "cima_direita") {
    servoEsq.write(130);
    servoDir.write(frenteDir);
  } else if (direcao == "baixo_esquerda") {
    servoEsq.write(trasEsq);
    servoDir.write(130);
  } else if (direcao == "baixo_direita") {
    servoEsq.write(50);
    servoDir.write(trasDir);
  } else {
    servoEsq.write(parado);
    servoDir.write(parado);
  }
  Serial.println("Movendo: " + direcao);
}

// função do movimento da garra 
void movGarra (int state) {
  if (state == 1){
    garra.write(30);
    Serial.println("garra: abrindo");
  } else if (state == 0){
    garra.write(150);
    Serial.println("garra: fechando");
  }
}

// Callback: recebe dados do ESP-NOW
void onReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  if (len == sizeof(JoystickState)) {
    memcpy(&jsState, incomingData, sizeof(JoystickState));
    // controle da garra
    if (jsState.botaoAbre) movGarra (1);
    else if (jsState.botaoFecha) movGarra (0);

    //controle do movimento
    if (jsState.cima && jsState.esquerda) mover("cima_esquerda");
    else if (jsState.cima && jsState.direita) mover("cima_direita");
    else if (jsState.baixo && jsState.esquerda) mover("baixo_esquerda");
    else if (jsState.baixo && jsState.direita) mover("baixo_direita");
    else if (jsState.cima) mover("frente");
    else if (jsState.baixo) mover("tras");
    else if (jsState.esquerda) mover("esquerda");
    else if (jsState.direita) mover("direita");
    else mover("parar");

  }
}

void setup() {
  Serial.begin(115200);

  //-- inicialização do esp-NOW --
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Erro inicializando ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onReceive);
  //-- fim da inicialização do esp-NOW--
  
  // define as portas usadas para comunicar com os servos
  servoEsq.attach(D3);
  servoDir.attach(D4);
  garra.attach(D6);

  // deixa o openDecabot inicialmente parado
  mover("parar");

  pinMode(buzzer, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  matrix.setRotation(2);
}

void loop() {
  // olhos piscando
  matrix.clear();
  matrix.drawLine(lpos, 2, lpos, 7, LED_ON);
  matrix.drawLine(rpos, 2, rpos, 7, LED_ON);
  matrix.writeDisplay();
  delay(1000);

  // piscada
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

  // olhar aleatório
  aleat = random(10);
  if (aleat < 2) { lpos = 0; rpos = 5; }
  else if (aleat > 7) { lpos = 2; rpos = 7; }
  else { lpos = 1; rpos = 6; }
}