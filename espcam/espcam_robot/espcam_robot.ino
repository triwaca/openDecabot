#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"
#include "esp_arduino_version.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ================================================================
// CONFIGURAÇÕES DO PONTO DE ACESSO (SoftAP)
// Conecte o celular diretamente nesta rede Wi-Fi!
// ================================================================
const char* ap_ssid = "DECABOT-CAM";
const char* ap_password = "12345678";

// ================================================================
// PINOS DA ESP32-CAM (AI-THINKER)
// ================================================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// LEDs da placa
#define FLASH_LED_PIN      4   // Flash LED branco
#define ONBOARD_LED_PIN   33   // LED vermelho de status


// ================================================================
// PINOS DOS MOTORES E BUZZER (ESP32-CAM ENCAIXADA NO SOQUETE DA SHIELD)
//
// Mapeamento do soquete Wemos -> ESP32-CAM:
//   D0 -> GPIO3   | D1 -> GPIO14 | D2 -> GPIO15
//   D3 -> GPIO13  | D4 -> GPIO12 | D8 -> GPIO16
//   D6 -> GND (inutilizável, por isso o servo direito foi para o D3)
//
// ATENÇÃO: GPIO 16 é o CS da PSRAM. O buzzer (D8) só funciona com a
// PSRAM DESATIVADA na IDE; caso contrário o buzzer é ignorado.
// ================================================================
#define PIN_SERVO_ESQ     13   // Pino D0 da shield (Servo Esquerdo 360)
#define PIN_SERVO_DIR     3   // Pino D3 da shield (Servo Direito 360)
#define PIN_SERVO_GARRA   12   // Pino D4 da shield (Servo da Garra)
#define PIN_BUZZER        16   // Pino D8 da shield (Buzzer)

bool buzzerOk = false;     // Só fica true se a PSRAM estiver desativada

// ================================================================
// PARÂMETROS PWM DOS SERVOS 360° (50Hz - Período de 20ms)
// Resolução de 16 bits (0 a 65535)
// ================================================================
#define SERVO_FREQ               50
#define SERVO_RES                16
#define SERVO_PARADO             4915  // ~1500us
#define SERVO_HORARIO            6554  // ~2000us (Velocidade máxima)
#define SERVO_ANTI_HORARIO       3277  // ~1000us (Velocidade máxima)
#define SERVO_HORARIO_MEIO       5734  // ~1750us (Velocidade média p/ diagonais)
#define SERVO_ANTI_HORARIO_MEIO  4096  // ~1250us (Velocidade média p/ diagonais)

// ================================================================
// CALIBRAÇÃO DE LINHA RETA (TRIM)
// Se puxar para a ESQUERDA: aumente este valor (desacelera a roda direita).
// Se puxar para a DIREITA:  diminua este valor (ou coloque 0).
// Valores típicos para testar: 150 a 400 (iniciamos com 250).
// ================================================================
#define COMPENSACAO_RODA_DIREITA  300

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;
bool flashState = false;
int posGarra = 90; // Faixa configurada de 90 a 180 graus

// ================================================================
// CONTROLE DOS MOTORES E DA GARRA
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
  angulo = constrain(angulo, 90, 180);
  posGarra = angulo;

  // Mapeia o ângulo (90° a 180°) para o pulso PWM
  int duty = map(angulo, 0, 180, 1638, 8192);
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(PIN_SERVO_GARRA, duty);
#else
  ledcWrite(4, duty);
#endif
  Serial.printf("[GARRA] Ângulo: %d° | Duty: %d\n", angulo, duty);
}

void parar() {
  setServoEsq(SERVO_PARADO);
  setServoDir(SERVO_PARADO);
}

void frente() {
  // Motores em lados opostos giram em sentidos invertidos
  setServoEsq(SERVO_HORARIO);
  // Aplica a compensação na roda direita para corrigir o desvio para a esquerda
  setServoDir(SERVO_ANTI_HORARIO + COMPENSACAO_RODA_DIREITA);
}

void re() {
  setServoEsq(SERVO_ANTI_HORARIO);
  setServoDir(SERVO_HORARIO - COMPENSACAO_RODA_DIREITA);
}

void virarEsquerda() {
  setServoEsq(SERVO_ANTI_HORARIO);
  setServoDir(SERVO_ANTI_HORARIO);
}

void virarDireita() {
  setServoEsq(SERVO_HORARIO);
  setServoDir(SERVO_HORARIO);
}

// Diagonais (Curvas suaves combinadas)
void frenteEsquerda() {
  setServoEsq(SERVO_HORARIO_MEIO);
  setServoDir(SERVO_ANTI_HORARIO);
}

void frenteDireita() {
  setServoEsq(SERVO_HORARIO);
  setServoDir(SERVO_ANTI_HORARIO_MEIO);
}

void reEsquerda() {
  setServoEsq(SERVO_ANTI_HORARIO_MEIO);
  setServoDir(SERVO_HORARIO);
}

void reDireita() {
  setServoEsq(SERVO_ANTI_HORARIO);
  setServoDir(SERVO_HORARIO_MEIO);
}

void tocarBuzzer(int duracaoMs) {
  if (!buzzerOk) return;   // GPIO 16 em uso pela PSRAM: não mexer no pino
  digitalWrite(PIN_BUZZER, HIGH);
  delay(duracaoMs);
  digitalWrite(PIN_BUZZER, LOW);
}

// ================================================================
// STREAMING DE VÍDEO (MJPEG)
// ================================================================
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) return res;

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      res = ESP_FAIL;
      break;
    }
    _jpg_buf_len = fb->len;
    _jpg_buf = fb->buf;

    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }

    esp_camera_fb_return(fb);
    fb = NULL;
    _jpg_buf = NULL;

    if (res != ESP_OK) break;
  }
  return res;
}

// ================================================================
// HANDLER: COMANDOS DO ROBÔ (AJAX)
// ================================================================
static esp_err_t cmd_handler(httpd_req_t *req) {
  char buf[64];
  if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) {
    char val[16];
    if (httpd_query_key_value(buf, "go", val, sizeof(val)) == ESP_OK) {
      Serial.printf("[WEB CMD] Comando: %s\n", val);
      if (strcmp(val, "frente") == 0) frente();
      else if (strcmp(val, "re") == 0) re();
      else if (strcmp(val, "esq") == 0) virarEsquerda();
      else if (strcmp(val, "dir") == 0) virarDireita();
      else if (strcmp(val, "frente_esq") == 0) frenteEsquerda();
      else if (strcmp(val, "frente_dir") == 0) frenteDireita();
      else if (strcmp(val, "re_esq") == 0) reEsquerda();
      else if (strcmp(val, "re_dir") == 0) reDireita();
      else if (strcmp(val, "parar") == 0) parar();
      else if (strcmp(val, "buzzer") == 0) tocarBuzzer(150);
      else if (strcmp(val, "flash") == 0) {
        flashState = !flashState;
        digitalWrite(FLASH_LED_PIN, flashState ? HIGH : LOW);
      }
      else if (strcmp(val, "garra_cima") == 0) setGarra(posGarra + 3);
      else if (strcmp(val, "garra_baixo") == 0) setGarra(posGarra - 3);
      else if (strcmp(val, "garra_abrir") == 0) setGarra(90);
      else if (strcmp(val, "garra_fechar") == 0) setGarra(180);
    }
    // Controle analógico contínuo da garra
    char gval[16];
    if (httpd_query_key_value(buf, "garra", gval, sizeof(gval)) == ESP_OK) {
      int ang = atoi(gval);
      Serial.printf("[WEB CMD] Garra angulo: %d\n", ang);
      setGarra(ang);
    }
  }
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, "OK", 2);
}

// ================================================================
// HANDLER: INTERFACE WEB DO CONTROLADOR (DESIGN MODERNO / COCKPIT)
// ================================================================
static const char PROGMEM CONTROLLER_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Decabot FPV Cockpit</title>
  <style>
    * {
      box-sizing: border-box;
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      user-select: none;
      margin: 0;
      padding: 0;
    }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      background: radial-gradient(circle at top, #131b2e 0%, #070a12 100%);
      color: #e2e8f0;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 8px 12px 18px 12px;
      touch-action: manipulation;
    }

    /* BARRA SUPERIOR HUD */
    .hud-header {
      width: 100%;
      max-width: 440px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 8px;
      padding: 2px 4px;
    }
    .hud-title {
      font-size: 1.05rem;
      font-weight: 800;
      letter-spacing: 1.5px;
      color: #38bdf8;
      display: flex;
      align-items: center;
      gap: 8px;
      text-transform: uppercase;
      text-shadow: 0 0 10px rgba(56, 189, 248, 0.5);
    }
    .status-dot {
      width: 10px;
      height: 10px;
      background: #10b981;
      border-radius: 50%;
      box-shadow: 0 0 10px #10b981;
      animation: pulse 1.5s infinite;
    }
    @keyframes pulse {
      0% { transform: scale(0.95); opacity: 0.8; }
      50% { transform: scale(1.15); opacity: 1; box-shadow: 0 0 14px #10b981; }
      100% { transform: scale(0.95); opacity: 0.8; }
    }
    .hud-badge {
      font-size: 11px;
      font-weight: bold;
      background: rgba(56, 189, 248, 0.12);
      border: 1px solid rgba(56, 189, 248, 0.3);
      color: #7dd3fc;
      padding: 3px 8px;
      border-radius: 20px;
      letter-spacing: 0.5px;
    }

    /* MOLDURA DA CÂMERA */
    .camera-frame {
      width: 100%;
      max-width: 440px;
      background: #000;
      border-radius: 16px;
      position: relative;
      overflow: hidden;
      border: 1.5px solid rgba(56, 189, 248, 0.35);
      box-shadow: 0 8px 30px rgba(0, 0, 0, 0.8), 0 0 15px rgba(56, 189, 248, 0.15);
    }
    .camera-frame img {
      width: 100%;
      height: 230px;
      object-fit: cover;
      display: block;
    }
    .hud-overlay-rec {
      position: absolute;
      top: 10px;
      left: 12px;
      display: flex;
      align-items: center;
      gap: 6px;
      font-size: 11px;
      font-weight: bold;
      color: #ef4444;
      background: rgba(0, 0, 0, 0.6);
      padding: 3px 8px;
      border-radius: 6px;
      letter-spacing: 1px;
      backdrop-filter: blur(4px);
    }
    .rec-dot {
      width: 7px;
      height: 7px;
      background: #ef4444;
      border-radius: 50%;
    }

    /* CONTAINER PRINCIPAL: DPAD + CONTROLE LATERAL DA GARRA */
    .controls-wrapper {
      margin-top: 14px;
      display: flex;
      justify-content: center;
      align-items: center;
      gap: 14px;
      width: 100%;
      max-width: 440px;
      touch-action: none;
    }

    /* GRID DIRECIONAL COM DIAGONAIS (3x3) */
    .dpad-grid {
      display: grid;
      grid-template-columns: repeat(3, 70px);
      grid-template-rows: repeat(3, 70px);
      gap: 8px;
    }
    .btn-pad {
      background: linear-gradient(145deg, #1e293b, #0f172a);
      border: 1.5px solid #334155;
      border-radius: 16px;
      color: #94a3b8;
      font-size: 24px;
      display: flex;
      justify-content: center;
      align-items: center;
      cursor: pointer;
      box-shadow: 0 4px 0 #020617, 0 6px 12px rgba(0,0,0,0.4);
      transition: all 0.08s ease;
      touch-action: none;
    }
    .btn-cardinal {
      color: #f8fafc;
      border-color: #38bdf8;
      background: linear-gradient(145deg, #0369a1, #0c4a6e);
      box-shadow: 0 4px 0 #082f49, 0 6px 14px rgba(3, 105, 161, 0.3);
    }
    .btn-diag {
      color: #38bdf8;
      font-size: 22px;
      border-color: rgba(56, 189, 248, 0.4);
      background: linear-gradient(145deg, #1e293b, #111e38);
    }
    .btn-stop {
      background: linear-gradient(145deg, #dc2626, #991b1b);
      border-color: #f87171;
      color: #fff;
      font-weight: 900;
      font-size: 14px;
      letter-spacing: 1px;
      box-shadow: 0 4px 0 #7f1d1d, 0 6px 16px rgba(220, 38, 38, 0.4);
    }
    .btn-pad:active, .btn-pad.active {
      transform: translateY(3px);
      box-shadow: 0 1px 0 #000, 0 0 12px rgba(56, 189, 248, 0.6);
      filter: brightness(1.2);
    }
    .btn-stop:active, .btn-stop.active {
      box-shadow: 0 1px 0 #000, 0 0 15px rgba(239, 68, 68, 0.8);
      background: #b91c1c;
    }

    /* ÍCONES DE SETA VETORIAIS (SEM EMOJIS) */
    .btn-pad svg {
      width: 28px;
      height: 28px;
      stroke: currentColor;
      stroke-width: 2.6;
      stroke-linecap: round;
      stroke-linejoin: round;
      fill: none;
      display: block;
      pointer-events: none;
    }
    .btn-claw svg {
      width: 14px;
      height: 14px;
      stroke: currentColor;
      stroke-width: 2.8;
      stroke-linecap: round;
      stroke-linejoin: round;
      fill: none;
      display: block;
      pointer-events: none;
      margin-bottom: 2px;
    }

    /* PAINEL LATERAL EXCLUSIVO PARA CONTROLE DA GARRA */
    .claw-lateral-panel {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: space-between;
      background: linear-gradient(145deg, #1e293b, #0f172a);
      border: 1.5px solid rgba(56, 189, 248, 0.35);
      border-radius: 18px;
      padding: 8px 6px;
      height: 226px;
      width: 76px;
      box-shadow: 0 4px 0 #020617, 0 6px 14px rgba(0,0,0,0.5);
    }
    .claw-header {
      font-size: 10px;
      font-weight: 900;
      letter-spacing: 1px;
      color: #38bdf8;
      text-transform: uppercase;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 2px;
    }
    .claw-deg-badge {
      font-size: 11px;
      font-weight: 800;
      color: #a5f3fc;
      background: rgba(6, 182, 212, 0.2);
      border: 1px solid rgba(56, 189, 248, 0.3);
      padding: 1px 6px;
      border-radius: 8px;
      min-width: 40px;
      text-align: center;
    }
    .btn-claw {
      background: linear-gradient(145deg, #0284c7, #0369a1);
      border: 1.5px solid #38bdf8;
      border-radius: 12px;
      color: white;
      width: 64px;
      height: 48px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: 0 3px 0 #075985;
      touch-action: none;
      transition: all 0.08s ease;
      user-select: none;
      -webkit-user-select: none;
    }
    .btn-claw:active, .btn-claw.active {
      transform: translateY(2px);
      box-shadow: 0 1px 0 #075985;
      filter: brightness(1.25);
      background: linear-gradient(145deg, #38bdf8, #0284c7);
    }
    .btn-claw svg {
      width: 20px;
      height: 20px;
      stroke: currentColor;
      stroke-width: 2.8;
      stroke-linecap: round;
      stroke-linejoin: round;
      fill: none;
      display: block;
      pointer-events: none;
    }
    .btn-claw span {
      font-size: 9px;
      font-weight: 800;
      letter-spacing: 0.5px;
      pointer-events: none;
    }
    .slider-track-wrap {
      height: 60px;
      display: flex;
      align-items: center;
      justify-content: center;
      width: 100%;
    }
    .claw-v-slider {
      -webkit-appearance: none;
      appearance: none;
      width: 58px;
      height: 6px;
      background: #334155;
      border-radius: 6px;
      outline: none;
      transform: rotate(-90deg);
      cursor: pointer;
      touch-action: none;
    }
    .claw-v-slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #38bdf8;
      border: 2px solid #ffffff;
      box-shadow: 0 0 8px #38bdf8;
      cursor: pointer;
    }
    .claw-v-slider::-moz-range-thumb {
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #38bdf8;
      border: 2px solid #ffffff;
      box-shadow: 0 0 8px #38bdf8;
      cursor: pointer;
    }

    /* BARRA DE AÇÕES AUXILIARES */
    .aux-bar {
      display: flex;
      gap: 12px;
      margin-top: 14px;
      width: 100%;
      max-width: 320px;
    }
    .btn-action {
      flex: 1;
      padding: 12px 10px;
      border-radius: 12px;
      font-size: 13px;
      font-weight: 800;
      letter-spacing: 0.8px;
      border: 1.5px solid transparent;
      color: white;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 6px;
      cursor: pointer;
      box-shadow: 0 4px 0 rgba(0,0,0,0.6);
      transition: all 0.08s ease;
      touch-action: none;
    }
    .btn-buzzer {
      background: linear-gradient(145deg, #d97706, #b45309);
      border-color: #fcd34d;
      box-shadow: 0 4px 0 #78350f, 0 6px 14px rgba(217, 119, 6, 0.3);
    }
    .btn-flash {
      background: linear-gradient(145deg, #4f46e5, #3730a3);
      border-color: #a5b4fc;
      box-shadow: 0 4px 0 #312e81, 0 6px 14px rgba(79, 70, 229, 0.3);
    }
    .btn-action:active, .btn-action.active {
      transform: translateY(3px);
      box-shadow: 0 1px 0 rgba(0,0,0,0.6);
      filter: brightness(1.2);
    }
  </style>
</head>
<body>

  <!-- CABEÇALHO HUD -->
  <div class="hud-header">
    <div class="hud-title">
      <div class="status-dot"></div>
      DECABOT FPV
    </div>
    <div class="hud-badge">AP: 192.168.4.1</div>
  </div>

  <!-- VISOR DA CÂMERA -->
  <div class="camera-frame">
    <div class="hud-overlay-rec">
      <div class="rec-dot"></div> LIVE
    </div>
    <img id="streamImg" src="" alt="Vídeo Stream Decabot">
  </div>

  <!-- CONTAINER DE CONTROLES (DPAD + CONTROLE LATERAL DA GARRA) -->
  <div class="controls-wrapper">
    <!-- GRID DIRECIONAL 3x3 (SOMENTE SETAS VETORIAIS) -->
    <div class="dpad-grid">
      <!-- Linha 1 -->
      <button class="btn-pad btn-diag" data-cmd="frente_esq" aria-label="Frente Esquerda">
        <svg viewBox="0 0 24 24"><path d="M17 17L7 7M7 17V7h10"/></svg>
      </button>
      <button class="btn-pad btn-cardinal" data-cmd="frente" aria-label="Frente">
        <svg viewBox="0 0 24 24"><path d="M12 19V5M5 12l7-7 7 7"/></svg>
      </button>
      <button class="btn-pad btn-diag" data-cmd="frente_dir" aria-label="Frente Direita">
        <svg viewBox="0 0 24 24"><path d="M7 17L17 7M7 7h10v10"/></svg>
      </button>

      <!-- Linha 2 -->
      <button class="btn-pad btn-cardinal" data-cmd="esq" aria-label="Girar Esquerda">
        <svg viewBox="0 0 24 24"><path d="M19 12H5M12 19l-7-7 7-7"/></svg>
      </button>
      <button class="btn-pad btn-stop" data-cmd="parar" aria-label="Parar">
        <svg viewBox="0 0 24 24"><rect x="6" y="6" width="12" height="12" rx="2" fill="currentColor"/></svg>
      </button>
      <button class="btn-pad btn-cardinal" data-cmd="dir" aria-label="Girar Direita">
        <svg viewBox="0 0 24 24"><path d="M5 12h14M12 5l7 7-7 7"/></svg>
      </button>

      <!-- Linha 3 -->
      <button class="btn-pad btn-diag" data-cmd="re_esq" aria-label="Ré Esquerda">
        <svg viewBox="0 0 24 24"><path d="M17 7L7 17M17 17H7V7"/></svg>
      </button>
      <button class="btn-pad btn-cardinal" data-cmd="re" aria-label="Ré">
        <svg viewBox="0 0 24 24"><path d="M12 5v14M19 12l-7 7-7-7"/></svg>
      </button>
      <button class="btn-pad btn-diag" data-cmd="re_dir" aria-label="Ré Direita">
        <svg viewBox="0 0 24 24"><path d="M7 7l10 10M7 17h10V7"/></svg>
      </button>
    </div>

    <!-- CONTROLE LATERAL DA GARRA (FAIXA 90° A 180° SEM EMOJIS) -->
    <div class="claw-lateral-panel">
      <div class="claw-header">
        <span>GARRA</span>
        <span class="claw-deg-badge" id="clawDegText">90°</span>
      </div>

      <button class="btn-claw" id="btnClawUp" aria-label="Aumentar Ângulo">
        <svg viewBox="0 0 24 24"><path d="M12 19V5M5 12l7-7 7 7"/></svg>
        <span>CIMA</span>
      </button>

      <div class="slider-track-wrap">
        <input type="range" class="claw-v-slider" id="clawSlider" min="90" max="180" value="90">
      </div>

      <button class="btn-claw" id="btnClawDown" aria-label="Diminuir Ângulo">
        <svg viewBox="0 0 24 24"><path d="M12 5v14M19 12l-7 7-7-7"/></svg>
        <span>BAIXO</span>
      </button>
    </div>
  </div>

  <!-- BOTÕES DE AÇÃO: BUZINA & FLASH (SEM EMOJIS) -->
  <div class="aux-bar">
    <button class="btn-action btn-buzzer" id="btnBuzzer">BUZINA</button>
    <button class="btn-action btn-flash" id="btnFlash">FAROL</button>
  </div>

  <script>
    let currentAngle = 90;
    let clawTimer = null;

    function sendCmd(cmd) {
      fetch('/cmd?go=' + cmd).catch(() => {});
    }

    function setClawAngle(angle) {
      angle = Math.max(90, Math.min(180, Math.round(Number(angle))));
      currentAngle = angle;
      document.getElementById('clawDegText').innerText = angle + '°';
      const slider = document.getElementById('clawSlider');
      if (slider && Number(slider.value) !== angle) {
        slider.value = angle;
      }
      fetch('/cmd?garra=' + angle).catch(() => {});
    }

    function stepClaw(delta) {
      vibrate(15);
      setClawAngle(currentAngle + delta);
    }

    function startClaw(step) {
      stepClaw(step);
      clawTimer = setInterval(() => {
        if ((step > 0 && currentAngle < 180) || (step < 0 && currentAngle > 90)) {
          setClawAngle(currentAngle + step);
        }
      }, 75);
    }

    function stopClaw() {
      if (clawTimer) {
        clearInterval(clawTimer);
        clawTimer = null;
      }
    }

    function vibrate(ms) {
      if ('vibrate' in navigator) {
        navigator.vibrate(ms);
      }
    }

    // Botões Direcionais
    const dpadButtons = document.querySelectorAll('.btn-pad');
    dpadButtons.forEach(btn => {
      const cmd = btn.getAttribute('data-cmd');

      const startAction = (e) => {
        e.preventDefault();
        btn.classList.add('active');
        vibrate(20);
        sendCmd(cmd);
      };

      const stopAction = (e) => {
        e.preventDefault();
        btn.classList.remove('active');
        if (cmd !== 'parar') sendCmd('parar');
      };

      btn.addEventListener('pointerdown', startAction);
      btn.addEventListener('pointerup', stopAction);
      btn.addEventListener('pointercancel', stopAction);
      btn.addEventListener('pointerleave', stopAction);
    });

    // Slider Vertical da Garra
    const clawSlider = document.getElementById('clawSlider');
    if (clawSlider) {
      clawSlider.addEventListener('input', (e) => {
        setClawAngle(e.target.value);
      });
    }

    // Controle Lateral da Garra - Botões Cima e Baixo
    const btnClawUp = document.getElementById('btnClawUp');
    const btnClawDown = document.getElementById('btnClawDown');

    btnClawUp.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      btnClawUp.classList.add('active');
      startClaw(+2);
    });
    btnClawUp.addEventListener('pointerup', () => {
      btnClawUp.classList.remove('active');
      stopClaw();
    });
    btnClawUp.addEventListener('pointercancel', () => {
      btnClawUp.classList.remove('active');
      stopClaw();
    });
    btnClawUp.addEventListener('pointerleave', () => {
      btnClawUp.classList.remove('active');
      stopClaw();
    });

    btnClawDown.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      btnClawDown.classList.add('active');
      startClaw(-2);
    });
    btnClawDown.addEventListener('pointerup', () => {
      btnClawDown.classList.remove('active');
      stopClaw();
    });
    btnClawDown.addEventListener('pointercancel', () => {
      btnClawDown.classList.remove('active');
      stopClaw();
    });
    btnClawDown.addEventListener('pointerleave', () => {
      btnClawDown.classList.remove('active');
      stopClaw();
    });

    // Botão Buzzer
    const btnBuzzer = document.getElementById('btnBuzzer');
    btnBuzzer.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      btnBuzzer.classList.add('active');
      vibrate(40);
      sendCmd('buzzer');
    });
    btnBuzzer.addEventListener('pointerup', () => btnBuzzer.classList.remove('active'));

    // Botão Flash
    const btnFlash = document.getElementById('btnFlash');
    btnFlash.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      btnFlash.classList.add('active');
      vibrate(30);
      sendCmd('flash');
    });
    btnFlash.addEventListener('pointerup', () => btnFlash.classList.remove('active'));

    // Inicia o vídeo na porta 81 (assim a porta 80 fica 100% livre para os comandos dos motores)
    window.addEventListener('load', () => {
      const streamImg = document.getElementById('streamImg');
      if (streamImg) {
        streamImg.src = 'http://' + window.location.hostname + ':81/stream';
      }
    });
  </script>
</body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html; charset=utf-8");
  return httpd_resp_send(req, CONTROLLER_HTML, strlen(CONTROLLER_HTML));
}

// ================================================================
// INICIALIZAÇÃO DO SERVIDOR WEB (DUPLA PORTA: 80 P/ COMANDOS, 81 P/ STREAM)
// ================================================================
void startWebControlServer() {
  // 1. Servidor de Comandos e Interface Web (Porta 80)
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  config.ctrl_port = 32768;

  httpd_uri_t index_uri = { .uri = "/",    .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL };
  httpd_uri_t cmd_uri   = { .uri = "/cmd", .method = HTTP_GET, .handler = cmd_handler,   .user_ctx = NULL };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    Serial.println("[HTTP] Servidor de Comandos pronto na porta 80!");
  } else {
    Serial.println("[ERRO] Falha ao iniciar Servidor de Comandos na porta 80!");
  }

  // 2. Servidor de Transmissão de Vídeo (Porta 81 - Tarefa dedicada para não bloquear comandos)
  httpd_config_t stream_config = HTTPD_DEFAULT_CONFIG();
  stream_config.server_port = 81;
  stream_config.ctrl_port = 32769;

  httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL };

  if (httpd_start(&stream_httpd, &stream_config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("[HTTP] Servidor de Video Streaming pronto na porta 81!");
  } else {
    Serial.println("[ERRO] Falha ao iniciar Servidor de Video na porta 81!");
  }
}

// ================================================================
// SETUP
// ================================================================
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n==========================================");
  Serial.println("         DECABOT ESP32-CAM INICIANDO       ");
  Serial.println("==========================================");

  // 1. Inicializa a Câmera PRIMEIRO (para garantir posse do Timer 0 / 20MHz)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size   = FRAMESIZE_QVGA; // 320x240 (Muito rápido para pilotagem)
    config.jpeg_quality = 12;
    config.fb_count     = 2;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.grab_mode    = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size   = FRAMESIZE_QVGA;
    config.jpeg_quality = 14;
    config.fb_count     = 1;
    config.fb_location  = CAMERA_FB_IN_DRAM;
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERRO] Falha ao iniciar câmera: 0x%x\n", err);
  } else {
    Serial.println("[CAMERA] Câmera OK!");
  }

  // 2. Configuração dos pinos dos LEDs e Buzzer
  pinMode(FLASH_LED_PIN, OUTPUT);
  pinMode(ONBOARD_LED_PIN, OUTPUT);

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
  
  // Mantém o Flash LED desligado (GPIO 4) e acende apenas o LED vermelho de status (GPIO 33)
  digitalWrite(FLASH_LED_PIN, LOW);    // Flash LED frontal desligado
  digitalWrite(ONBOARD_LED_PIN, LOW);  // LED vermelho traseiro de status (GPIO 33, ativo baixo)

  // 3. Inicialização PWM dos Servos (Tração 360° e Garra)
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
  parar();       // Inicia com motores de tração parados
  setGarra(90);  // Inicia a garra em 90° (faixa de 90° a 180°)

  // Beep inicial de boas-vindas
  tocarBuzzer(100);

  // Cria o Ponto de Acesso Wi-Fi próprio
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress IP = WiFi.softAPIP();
  startWebControlServer();

  Serial.println("\n==========================================");
  Serial.printf(" REDE WI-FI: %s\n", ap_ssid);
  Serial.printf(" SENHA:      %s\n", ap_password);
  Serial.printf(" CONTROLE NO CELULAR: http://%s\n", IP.toString().c_str());
  Serial.println("==========================================\n");

  // Dois beeps curtos indicando que o sistema está pronto
  tocarBuzzer(80);
  delay(100);
  tocarBuzzer(80);
}

// ================================================================
// LOOP
// ================================================================
void loop() {
  delay(10000);
}

