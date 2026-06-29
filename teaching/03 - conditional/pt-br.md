# Condicionais e Comunicação Serial no Wemos D1 Mini

Este repositório contém um guia prático projetado para introduzir estudantes de robótica, automação e sistemas embarcados aos conceitos de **Comunicação Serial** e **Estruturas Condicionais (`if`, `else if`, `else`)** utilizando a placa **Wemos D1 Mini (ESP8266)** e a plataforma Arduino.

## 📋 Pré-requisitos

Antes de iniciar, certifique-se de ter os seguintes itens configurados:

### Hardware

* 1x Placa Wemos D1 Mini (ou qualquer módulo baseado no ESP8266)

* 1x Cabo de dados Micro-USB
  
  ### Software

* **Arduino IDE** (versão 1.8.x ou 2.x)

* Placas da família **ESP8266 instaladas** na IDE (Gerenciador de Placas)

* Drivers do chip de comunicação USB instalados (geralmente CH340)

---

## ⚡ Passo 1: O que é a Comunicação Serial?

A **Comunicação Serial** funciona como uma ponte de texto entre o microcontrolador e o seu computador. Em projetos de desenvolvimento, ela é a nossa principal ferramenta de diagnóstico (debug), permitindo-nos "espiar" o que está acontecendo dentro do chip.
No ecossistema Arduino, iniciamos essa comunicação usando o comando:

```cpp
Serial.begin(velocidade);
```

> [!IMPORTANT]
> **Por que usar 115200 bps no Wemos D1 Mini?**
> Diferente do Arduino Uno clássico (que costuma usar 9600), os microcontroladores da família ESP8266 inicializam o bootloader a uma taxa de **115200 bps**. Configurar o sistema nessa velocidade evita que você veja caracteres corrompidos ou ilegíveis (lixo de memória) na tela no momento em que a placa é resetada ou ligada.

### Comandos de Escrita Serial:

* `Serial.print("Texto");` -> Envia a informação e mantém o cursor na **mesma linha**.
* `Serial.println("Texto");` -> Envia a informação e **pula para a próxima linha** (adiciona uma quebra de linha `\n` ao final).

---

## 🔍 Passo 2: Como Visualizar os Dados na Arduino IDE

Para conseguir ler o que o Wemos D1 Mini está enviando, siga o procedimento abaixo:

1. Conecte o Wemos D1 Mini ao computador pelo cabo USB.
2. Na Arduino IDE, vá em **Ferramentas > Placa** e selecione `LOLIN(WEMOS) D1 R2 & mini` (ou `Generic ESP8266 Module`).
3. Vá em **Ferramentas > Porta** e selecione a porta COM associada à sua placa.
4. Clique no ícone do **Monitor Serial** (uma lupa no canto superior direito da IDE) ou pressione o atalho `Ctrl + Shift + M`.
5. **Ajuste essencial:** Na janela do Monitor Serial que se abriu, mude a velocidade no menu suspenso para **115200 baud**. Se estiver em outra velocidade, você não conseguirá ler o texto correto.

---

## 🧠 Passo 3: Entendendo as Estruturas Condicionais

As condicionais permitem que o microcontrolador execute ações diferentes dependendo do estado das variáveis ou sensores.

* **`if` (Se):** Avalia se uma condição é verdadeira. Se for, executa o bloco de código interno.
* **`else if` (Senão se):** Fornece uma nova condição para ser testada caso as anteriores tenham sido falsas.
* **`else` (Senão):** É a zona de escape. Executa um código padrão caso **nenhuma** das condições anteriores tenha sido atendida.

---

## 💻 Passo 4: Código Prático (Simulação de um Sensor de Distância)

Neste exemplo realístico, simulamos a lógica interna de um pequeno robô móvel que precisa desviar de obstáculos. A variável `distanciaObstaculo` decresce a cada ciclo para forçar o código a passar por todas as ramificações lógicas.
Copie o código abaixo, cole na sua Arduino IDE e clique em **Carregar (Upload)**:

```cpp
/**
 * @file Tutorial_Condicionais.ino
 * @brief Aprendendo condicionais e comunicação serial com o Wemos D1 Mini.
 * @author Professor / Instrutor
 */
// Variável global para simular a leitura de distância em centímetros (cm)
int distanciaObstaculo = 35; 
void setup() {
  // Inicializa a comunicação serial a 115200 bits por segundo (bps)
  Serial.begin(115200);

  // Pequeno intervalo de segurança para inicialização estável da placa
  delay(1000); 

  Serial.println("");
  Serial.println("=========================================");
  Serial.println("   SISTEMA DE DECISÃO LOGICA DO ROBÔ     ");
  Serial.println("=========================================");
}
void loop() {
  // Exibe a distância simulada atual no monitor
  Serial.print("Sensor de Distância: ");
  Serial.print(distanciaObstaculo);
  Serial.println(" cm");
  // --- INÍCIO DA TOMADA DE DECISÃO ---

  // Condição 1: Perigo imediato
  if (distanciaObstaculo <= 10) {
    Serial.println(" Alerta: [PERIGO] - Parar imediatamente! Risco de colisão.");
  }

  // Condição 2: Zona de atenção (Uso do operador lógico '&&' para verificar o intervalo)
  else if (distanciaObstaculo > 10 && distanciaObstaculo <= 30) {
    Serial.println(" Alerta: [ATENÇÃO] - Reduzir velocidade. Obstáculo próximo.");
  }

  // Condição 3: Caminho livre (Qualquer valor acima de 30)
  else {
    Serial.println(" Status: [LIVRE] - Velocidade máxima permitida.");
  }
  Serial.println("-----------------------------------------");
  // --- SIMULAÇÃO DA MUDANÇA DE ESTADO ---
  // Subtrai 8 cm da distância a cada execução para simular a aproximação do robô
  distanciaObstaculo = distanciaObstaculo - 8;
  // Se a distância se tornar negativa, reiniciamos a simulação para fins didáticos
  if (distanciaObstaculo < 0) {
    distanciaObstaculo = 40; 
    Serial.println("\n>>> [RESET] Reiniciando simulação de trajetória... <<<\n");
  }
  // Tempo de espera de 3 segundos para que os alunos possam ler confortavelmente
  delay(3000); 
}
```

---

## 📈 Resultado Esperado no Monitor Serial

Ao abrir o Monitor Serial configurado corretamente em `115200 baud`, os alunos deverão observar a seguinte sequência cíclica de mensagens de texto na tela:

```text
=========================================
   SISTEMA DE DECISÃO LOGICA DO ROBÔ     
=========================================
Sensor de Distância: 35 cm
 Status: [LIVRE] - Velocidade máxima permitida.
-----------------------------------------
Sensor de Distância: 27 cm
 Alerta: [ATENÇÃO] - Reduzir velocidade. Obstáculo próximo.
-----------------------------------------
Sensor de Distância: 19 cm
 Alerta: [ATENÇÃO] - Reduzir velocidade. Obstáculo próximo.
-----------------------------------------
Sensor de Distância: 11 cm
 Alerta: [ATENÇÃO] - Reduzir velocidade. Obstáculo próximo.
-----------------------------------------
Sensor de Distância: 3 cm
 Alerta: [PERIGO] - Parar imediatamente! Risco de colisão.
-----------------------------------------
>>> [RESET] Reiniciando simulação de trajetória... <<<
Sensor de Distância: 40 cm
 Status: [LIVRE] - Velocidade máxima permitida.
```

---

## 🚀 Desafios para Fixação (Prática em Laboratório)

Para garantir o aprendizado e testar o raciocínio lógico dos estudantes, proponha as seguintes modificações no código:

1. **Alerta Intermediário Exato:** Peça aos alunos para adicionarem uma condição usando `else if` que identifique quando a distância for exatamente `15` cm e imprima uma instrução específica no terminal, por exemplo: `" Status: [INFO] - Manobrar para a direita."` (Lembre-os de usar o operador de igualdade `==`).
2. **Alteração do Passo de Atualização:** Peça para alterarem o decremento do sensor (atualmente em `- 8`) para um valor menor, como `- 3`, e incentive-os a analisar o impacto desse valor na frequência com que os alertas mudam de estado.
3. **Uso de Operadores Relacionais:** Peça para modificarem o código para que o status `[PERIGO]` passe a cobrir qualquer valor menor ou igual a `15` cm, ajustando os intervalos lógicos subsequentes de forma coerente.

---

## 🛠️ Resolução de Problemas Comuns

* **O Monitor Serial exibe caracteres estranhos (`⸮⸮⸮` ou interrogações):** A velocidade do monitor está diferente de `115200`. Altere a caixa de seleção no canto inferior direito para o valor correto.
* **Erro ao carregar o código (Failed to connect to ESP8266):** O cabo USB pode ser apenas de carregamento (sem linhas de dados), a porta COM errada foi selecionada ou os drivers do chip CH340 precisam ser reinstalados.
