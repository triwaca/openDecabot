# Teste e Diagnóstico da ESP32-CAM (AI-Thinker)

Este projeto contém um código completo de teste e diagnóstico para o módulo **ESP32-CAM (OV2640)**.

O código foi projetado para ser compatível tanto com o **Arduino IDE** quanto com o **PlatformIO** (VS Code).

---

## 🔍 O que este teste verifica?

1. **Memória PSRAM**: Checa se a memória externa de 4MB está ativa e operando.
2. **LEDs Onboard**:
   - Pisca o **LED vermelho de status** (GPIO 33).
   - Aciona um pulso no **Flash LED de alta potência** (GPIO 4).
3. **Sensor de Câmera (OV2640)**:
   - Inicializa a comunicação com os pinos de dados e controle da câmera.
   - Captura um frame de teste em buffer para validar o sensor.
4. **Conexão Wi-Fi**: Conecta à sua rede e obtém um IP.
5. **Servidor Web com Streaming**:
   - Transmissão de vídeo ao vivo (MJPEG) direto no navegador.
   - Captura de foto individual (`/capture`).
   - Botão para ligar/desligar o Flash LED via interface web.

---

## 📂 Estrutura de Projetos Disponíveis

1. **`espcam_test/`**: Teste de diagnóstico inicial da ESP32-CAM (Câmera OV2640, PSRAM, LEDs e streaming Wi-Fi).
2. **`espcam_robot/`**: Código completo do Robô Decabot com controle via celular, vídeo FPV em tempo real, 8 direções (com diagonais), buzina e controle lateral da garra.
3. **`espcam_servo_test/`**: Código dedicado para testar e calibrar os 3 servos individualmente (Esquerdo 360°, Direito 360° e Garra 180°) com log no Monitor Serial.

---

## 🤖 Conexão com a Shield Decabot (Servos, Garra e Buzzer)

Para o robô com câmera e garra (`espcam_robot/espcam_robot.ino`):

- **Servo Esquerdo (360°):** Pino **`D0`** (Linha superior do bloco `S_ESQ`).
- **Servo Direito (360°):** Pino **`D3`** (Linha do meio do bloco `S_ESQ`).
- **Servo da Garra:** Pino **`D4`** (Linha do meio do bloco `S_DIR`).
- **Buzzer:** Pino **`D8`** (Linha superior do bloco `S_DIR`).

### Pinagem com a ESP32-CAM encaixada no soquete da shield:
- **GPIO 3**  ⬅ Pino **`D0`** (Servo Esquerdo 360°)
- **GPIO 13** ⬅ Pino **`D3`** (Servo Direito 360°; o `D6` cai no GND da ESP32-CAM)
- **GPIO 12** ⬅ Pino **`D4`** (Servo da Garra)
- **GPIO 16** ⬅ Pino **`D8`** (Buzzer; só funciona com a **PSRAM desativada**)
- **5V / GND** ⬅ Pinos **`5V` / `G`** do soquete

---

## ⚡ Esquema de Ligação para Gravação (FTDI / Conversor USB-TTL)

Como a ESP32-CAM não possui porta USB nativa, utilize um módulo FTDI ou conversor USB-Serial:

| ESP32-CAM | Conversor USB-Serial (FTDI) | Observação |
| :--- | :--- | :--- |
| **5V** | **5V / VCC** | **Muito importante**: use 5V (3.3V gera quedas de tensão e erro de Brownout) |
| **GND** | **GND** | Terra comum |
| **U0T** (TX) | **RX** | Pino TX cruza com RX |
| **U0R** (RX) | **TX** | Pino RX cruza com TX |
| **GPIO 0** | **GND** | **Obrigatório para gravar**: conecte ao GND antes de plugar na USB |

> ⚠️ **Atenção**: 
> 1. Para **GRAVAR (Upload)**: Deixe **GPIO 0 conectado ao GND** e aperte o botão RESET da placa.
> 2. Para **EXECUTAR (Testar)**: Remova o jumper/fio entre **GPIO 0 e GND** e aperte o botão RESET novamente.

---

## 🚀 Como Usar no Arduino IDE

1. Abra a pasta `espcam_test` e abra o arquivo `espcam_test.ino`.
## 🌐 Modo de Conexão: Ponto de Acesso (SoftAP)

A ESP32-CAM está configurada para gerar sua **própria rede Wi-Fi**, dispensando roteadores, internet ou redes públicas bloqueadas:

- **Nome da Rede (SSID):** `ESP32-CAM-TESTE`
- **Senha:** `12345678`
- **Endereço de Acesso no Navegador:** `http://192.168.4.1`

*(Você pode alterar o nome da rede e a senha no início do arquivo `espcam_test/espcam_test.ino`).*
3. No menu **Ferramentas (Tools)**, ajuste as configurações:
   - **Placa (Board)**: `AI Thinker ESP32-CAM`
   - **CPU Frequency**: `240MHz (WiFi/BT)`
   - **Flash Frequency**: `80MHz`
   - **Flash Mode**: `QIO`
   - **Partition Scheme**: `Huge APP (3MB No OTA/1MB SPIFFS)`
   - **PSRAM**: `Enabled`
   - **Porta (Port)**: Selecione a porta COM / `/dev/ttyUSBx` correspondente ao seu FTDI.
4. Clique em **Carregar (Upload)**.
5. Após o upload, desconecte o pino `GPIO 0` do `GND`, abra o **Monitor Serial** em **115200 baud** e aperte o botão **RESET** da placa.
6. Copie o endereço IP exibido no Monitor Serial (ex: `http://192.168.1.50`) e abra no navegador.

---

## 💻 Como Usar no PlatformIO

Se estiver utilizando VS Code com a extensão PlatformIO:

1. Abra a pasta raiz deste repositório (`espcam`).
2. Edite o SSID e senha em `espcam_test/espcam_test.ino`.
3. Conecte o pino `GPIO 0` ao `GND`.
4. Clique no ícone de **Upload** do PlatformIO.
5. Desconecte o `GPIO 0` do `GND` e abra o **Serial Monitor** (configurado para 115200 baud).

---

## 🛠️ Resolução de Problemas Comuns

- **`Camera probe failed with error 0x20002`**:
  - O cabo flat do sensor OV2640 está mal encaixado ou invertido. Abra a trava plástica preta do conector e recoloque o cabo com cuidado.
- **`Brownout detector was triggered` / A placa fica reiniciando em loop**:
  - Alimentação insuficiente. A ESP32-CAM consome picos de corrente de até 300-500mA ao transmitir via Wi-Fi e ativar o sensor. Forneça alimentação de 5V estável através de uma fonte externa ou porta USB de boa qualidade.
- **`A fatal error occurred: Failed to connect to ESP32: Timed out waiting for packet header`**:
  - O pino `GPIO 0` não estava conectado ao `GND` no momento do boot. Conecte ao GND e aperte o botão RESET antes de clicar em carregar.

