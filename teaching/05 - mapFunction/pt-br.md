# Tutorial Prático: Função `map()` e Leitura Analógica no Wemos D1 Mini

Este repositório contém um guia prático para ensinar como utilizar a função `map()` da Arduino IDE. Você aprenderá a ler um sensor analógico (potenciômetro) e converter seus dados brutos para diferentes escalas úteis (porcentagem, graus e escala invertida).

## 📋 Pré-requisitos

### Hardware

* 1x Placa Wemos D1 Mini (ESP8266)

* 1x Potenciômetro (ex: 10kΩ)

* 1x Protoboard

* 3x Jumpers (fios macho-macho)

* 1x Cabo Micro-USB de dados
  
  ### Software

* Arduino IDE instalada e configurada para placas ESP8266.

---

## 📖 Passo 1: A Teoria – O que é a função `map()`?

Na programação com Arduino, muitas vezes lemos um valor que está em uma **escala A** e precisamos convertê-lo proporcionalmente para uma **escala B**. A função `map()` faz essa "regra de três" automaticamente.
A sintaxe exige 5 parâmetros:

```cpp
resultado = map(valor_lido, de_min, de_max, para_min, para_max);
```

* **`valor_lido`**: Variável com o número original.

* **`de_min` e `de_max`**: Intervalo original (ex: o que o sensor gera).

* **`para_min` e `para_max`**: Intervalo de destino (ex: para onde você quer converter).
  **Exemplo prático:** O conversor analógico-digital (ADC) do Wemos D1 Mini lê tensões no pino `A0` e transforma em números de **0 a 1023**. Para converter essa leitura em uma porcentagem (de **0 a 100%**):
  
  ```cpp
  porcentagem = map(leitura, 0, 1023, 0, 100);
  ```

---

## 🔌 Passo 2: Montagem do Circuito (Protoboard)

O Wemos D1 Mini possui apenas um pino analógico, o **A0**. Vamos conectar o sinal do nosso potenciômetro nele.

> [!WARNING]
> **Cuidado com a Tensão:** O pino analógico (A0) do Wemos D1 Mini suporta no máximo **3.3V**. Nunca conecte o potenciômetro ao pino de 5V da placa, sob risco de danificar o microcontrolador permanentemente.

### Conexões:

1. Encaixe o potenciômetro na protoboard.
2. **Pino da Esquerda:** Conecte ao pino **3V3** do Wemos.
3. **Pino da Direita:** Conecte ao pino **G** (GND) do Wemos.
4. **Pino Central (Cursor):** Conecte ao pino **A0** do Wemos.
   *(Nota: Inverter as ligações dos pinos das extremidades apenas inverte a direção de leitura, mas não causa danos.)*

---

## 💻 Passo 3: O Código (Prática)

Neste código, vamos ler o valor bruto do potenciômetro (0 a 1023) e convertê-lo para três escalas diferentes.

```cpp
/**
 * @file Tutorial_Map_Potenciometro.ino
 * @brief Prática com função map() e potenciômetro no Wemos D1 Mini.
 */
// Define o pino onde o potenciômetro está conectado
const int pinoPotenciometro = A0; 
void setup() {
  // Inicia a comunicação serial a 115200 bps
  Serial.begin(115200);
  delay(1000); 

  Serial.println("\n--- Iniciando Leitura do Potenciômetro ---");
}
void loop() {
  // 1. Lê o valor bruto do ADC
  // No Wemos D1 Mini, esse valor vai de 0 a 1023
  int valorBruto = analogRead(pinoPotenciometro);
  // 2. Usando a função map() para diferentes aplicações

  // Exemplo A: Convertendo para porcentagem (0 a 100%)
  int porcentagem = map(valorBruto, 0, 1023, 0, 100);
  // Exemplo B: Convertendo para ângulo de um servo motor (0 a 180 graus)
  int anguloServo = map(valorBruto, 0, 1023, 0, 180);
  // Exemplo C: Escala invertida (ex: 10 até 0).
  int contagemRegressiva = map(valorBruto, 0, 1023, 10, 0);
  // 3. Imprimindo os resultados no Monitor Serial
  Serial.print("Bruto: ");
  Serial.print(valorBruto);

  Serial.print(" | Porcentagem: ");
  Serial.print(porcentagem);
  Serial.print("%");
  Serial.print(" | Angulo: ");
  Serial.print(anguloServo);
  Serial.print("°");
  Serial.print(" | Invertido: ");
  Serial.println(contagemRegressiva);
  // Aguarda meio segundo antes de fazer uma nova leitura
  delay(500); 
}
```

---

## 📈 Resultado Esperado no Monitor Serial

Ao abrir o Monitor Serial configurado corretamente em `115200 baud` e girar o potenciômetro, os alunos deverão observar algo parecido com isto:

```text
--- Iniciando Leitura do Potenciômetro ---
Bruto: 0 | Porcentagem: 0% | Angulo: 0° | Invertido: 10
Bruto: 256 | Porcentagem: 25% | Angulo: 45° | Invertido: 7
Bruto: 512 | Porcentagem: 50% | Angulo: 90° | Invertido: 5
Bruto: 1023 | Porcentagem: 100% | Angulo: 180° | Invertido: 0
```

---

## 🚀 Desafio para Fixação (Prática em Laboratório)

Para testar o conhecimento adquirido, tente resolver o seguinte desafio:

> Modifique o código acima para que a função `map()` simule um sensor de temperatura falso. Quando o potenciômetro estiver girado totalmente para a esquerda, o valor exibido deve ser **-20°C**. Quando estiver totalmente para a direita, deve ser **50°C**.
> **Dica:** Lembre-se de alterar os limites de destino (`para_min` e `para_max`) na sua função `map()`.
