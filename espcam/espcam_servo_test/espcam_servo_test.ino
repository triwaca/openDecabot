#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_arduino_version.h"

// ================================================================
// PINOS DOS SERVOS E BUZZER (ESP32-CAM ENCAIXADA NO SOQUETE DA SHIELD)
//
// Mapeamento do soquete Wemos -> ESP32-CAM:
//   D0 -> GPIO3   | D1 -> GPIO14 | D2 -> GPIO15
//   D3 -> GPIO13  | D4 -> GPIO12 | D8 -> GPIO16
//   D6 -> GND (inutilizável, por isso o servo direito foi para o D3)
//
// ATENÇÃO: GPIO 16 é o CS da PSRAM. O buzzer (D8) só funciona com a
// PSRAM DESATIVADA na IDE; caso contrário o buzzer é ignorado.
// ================================================================
#define PIN_SERVO_ESQ      3   // Pino D0 da shield (Servo Esquerdo 360)
#define PIN_SERVO_DIR     13   // Pino D3 da shield (Servo Direito 360)
#define PIN_SERVO_GARRA   12   // Pino D4 da shield (Servo da Garra)
#define PIN_BUZZER        16   // Pino D8 da shield (Buzzer)

bool buzzerOk = false;     // Só fica true se a PSRAM estiver desativada

// LEDs da ESP32-CAM
#define FLASH_LED_PIN      4   // Flash LED branco (permanecerá SEMPRE DESLIGADO)
#define ONBOARD_LED_PIN   33   // LED vermelho traseiro de status

// ================================================================
// PARÂMETROS PWM (50Hz - Período de 20ms - 16 bits: 0 a 65535)
// ================================================================
#define SERVO_FREQ               50
#define SERVO_RES                16
#define SERVO_PARADO             4915  // ~1500us (Neutro/Parado para servo 360°)
#define SERVO_HORARIO            6554  // ~2000us (Giro horário total)
#define SERVO_ANTI_HORARIO       3277  // ~1000us (Giro anti-horário total)

// Calibração de Linha Reta: reduz a velocidade da roda direita para não puxar à esquerda
#define COMPENSACAO_RODA_DIREITA  250

// ================================================================
// FUNÇÕES DE CONTROLE DOS MOTORES
// ================================================================
void setServoEsq(int duty) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(PIN_SERVO_ESQ, duty);
#else
  ledcWrite(2, duty);
#endif
}

void setServoDir(int duty) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(PIN_SERVO_DIR, duty);
#else
  ledcWrite(3, duty);
#endif
}

void setGarra(int angulo) {
  angulo = constrain(angulo, 0, 180);
  int duty = map(angulo, 0, 180, 1638, 8192);
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(PIN_SERVO_GARRA, duty);
#else
  ledcWrite(4, duty);
#endif
}

void pararTodos() {
  setServoEsq(SERVO_PARADO);
  setServoDir(SERVO_PARADO);
}

void beep(int ms) {
  if (!buzzerOk) return;   // GPIO 16 em uso pela PSRAM: não mexer no pino
  digitalWrite(PIN_BUZZER, HIGH);
  delay(ms);
  digitalWrite(PIN_BUZZER, LOW);
}

// ================================================================
// SETUP
// ================================================================
void setup() {
  // Desativa detector de brownout
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n==========================================");
  Serial.println("       TESTE DOS 3 SERVOS - DECABOT       ");
  Serial.println("==========================================");

  // Configuração dos pinos dos LEDs e Buzzer
  pinMode(FLASH_LED_PIN, OUTPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  
  // Flash SEMPRE DESLIGADO e LED vermelho aceso
  digitalWrite(FLASH_LED_PIN, LOW);
  digitalWrite(ONBOARD_LED_PIN, LOW); // LOW liga o LED vermelho traseiro

  // Buzzer no GPIO 16: só pode ser usado se a PSRAM estiver desativada
  if (psramFound()) {
    buzzerOk = false;
    Serial.println("[AVISO] PSRAM ativa: GPIO 16 (buzzer D8) desabilitado para evitar travamento.");
    Serial.println("        Para usar o buzzer: Ferramentas > PSRAM > Disabled (placa 'ESP32 Dev Module').");
  } else {
    buzzerOk = true;
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
  }

  // Inicialização do PWM dos Servos
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcAttach(PIN_SERVO_ESQ, SERVO_FREQ, SERVO_RES);
  ledcAttach(PIN_SERVO_DIR, SERVO_FREQ, SERVO_RES);
  ledcAttach(PIN_SERVO_GARRA, SERVO_FREQ, SERVO_RES);
#else
  ledcSetup(2, SERVO_FREQ, SERVO_RES);
  ledcAttachPin(PIN_SERVO_ESQ, 2);
  ledcSetup(3, SERVO_FREQ, SERVO_RES);
  ledcAttachPin(PIN_SERVO_DIR, 3);
  ledcSetup(4, SERVO_FREQ, SERVO_RES);
  ledcAttachPin(PIN_SERVO_GARRA, 4);
#endif

  pararTodos();
  setGarra(90);

  Serial.println("[INFO] Pinos configurados:");
  Serial.println("  -> Servo Esquerdo (360): GPIO 3  (Shield D0)");
  Serial.println("  -> Servo Direito (360):  GPIO 13 (Shield D3)");
  Serial.println("  -> Servo da Garra (180): GPIO 12 (Shield D4)");
  Serial.println("  -> Buzzer:               GPIO 16 (Shield D8)");
  Serial.println("  -> Flash LED:            DESLIGADO");

  // Beep curto indicando início do teste
  beep(80);
  delay(100);
  beep(80);
  delay(1000);
}

// ================================================================
// LOOP: CICLO DE TESTE COMPLETO DOS 3 SERVOS
// ================================================================
void loop() {
  Serial.println("\n------------------------------------------");
  Serial.println(">>> INICIANDO CICLO DE TESTES...");
  Serial.println("------------------------------------------");

  // ==============================================================
  // TESTE 1: SERVO ESQUERDO (360° - GPIO 3 / D0)
  // ==============================================================
  Serial.println("\n[TESTE 1] Servo Esquerdo (D0): Girando Sentido HORÁRIO...");
  setServoEsq(SERVO_HORARIO);
  delay(2000);

  Serial.println("[TESTE 1] Servo Esquerdo (D0): PARANDO...");
  setServoEsq(SERVO_PARADO);
  delay(1000);

  Serial.println("[TESTE 1] Servo Esquerdo (D0): Girando Sentido ANTI-HORÁRIO...");
  setServoEsq(SERVO_ANTI_HORARIO);
  delay(2000);

  Serial.println("[TESTE 1] Servo Esquerdo (D0): PARANDO...");
  setServoEsq(SERVO_PARADO);
  delay(1000);

  // ==============================================================
  // TESTE 2: SERVO DIREITO (360° - GPIO 13 / D3)
  // ==============================================================
  Serial.println("\n[TESTE 2] Servo Direito (D3): Girando Sentido HORÁRIO...");
  setServoDir(SERVO_HORARIO);
  delay(2000);

  Serial.println("[TESTE 2] Servo Direito (D3): PARANDO...");
  setServoDir(SERVO_PARADO);
  delay(1000);

  Serial.println("[TESTE 2] Servo Direito (D3): Girando Sentido ANTI-HORÁRIO...");
  setServoDir(SERVO_ANTI_HORARIO);
  delay(2000);

  Serial.println("[TESTE 2] Servo Direito (D3): PARANDO...");
  setServoDir(SERVO_PARADO);
  delay(1000);

  // ==============================================================
  // TESTE 3: OS DOIS SERVOS JUNTOS (SIMULAÇÃO DE TRAÇÃO)
  // ==============================================================
  Serial.println("\n[TESTE 3] Ambos os Servos: AVANÇAR (Frente)...");
  setServoEsq(SERVO_HORARIO);
  setServoDir(SERVO_ANTI_HORARIO + COMPENSACAO_RODA_DIREITA);
  delay(2000);

  Serial.println("[TESTE 3] Ambos os Servos: PARANDO...");
  pararTodos();
  delay(1000);

  Serial.println("[TESTE 3] Ambos os Servos: RECUAR (Ré)...");
  setServoEsq(SERVO_ANTI_HORARIO);
  setServoDir(SERVO_HORARIO - COMPENSACAO_RODA_DIREITA);
  delay(2000);

  Serial.println("[TESTE 3] Ambos os Servos: PARANDO...");
  pararTodos();
  delay(1500);

  // ==============================================================
  // TESTE 4: SERVO DA GARRA (180° - GPIO 12 / D4)
  // ==============================================================
  Serial.println("\n[TESTE 4] Garra (D4): Abrindo lentamente até 180 graus...");
  for (int angulo = 0; angulo <= 180; angulo += 10) {
    setGarra(angulo);
    delay(50);
  }
  Serial.println("[TESTE 4] Garra (D4): Totalmente ABERTA (180 graus)");
  delay(1500);

  Serial.println("[TESTE 4] Garra (D4): Fechando lentamente até 0 graus...");
  for (int angulo = 180; angulo >= 0; angulo -= 10) {
    setGarra(angulo);
    delay(50);
  }
  Serial.println("[TESTE 4] Garra (D4): Totalmente FECHADA (0 graus)");
  delay(1500);

  Serial.println("[TESTE 4] Garra (D4): Retornando ao ponto central (90 graus)...");
  setGarra(90);
  delay(1000);

  // Fim do ciclo com sinal sonoro
  Serial.println("\n[FIM DO CICLO] Todos os testes concluídos com sucesso!");
  beep(120);

  Serial.println("Aguardando 4 segundos para reiniciar o ciclo de testes...");
  delay(4000);
}

