# Tutorial Prático: Criando e Utilizando Funções no Arduino

Bem-vindos a este guia pedagógico sobre estruturação de código. Quando nossos programas começam a crescer — seja controlando um semáforo complexo, lendo múltiplos sensores ou gerenciando a movimentação de um robô como o Decabot —, colocar todo o código dentro das funções `setup()` e `loop()` torna o projeto confuso, difícil de depurar e propenso a erros.
As **funções** são blocos de código independentes e reutilizáveis que realizam tarefas específicas. Elas nos ajudam a modularizar o sistema, evitar a repetição de código (princípio DRY - *Don't Repeat Yourself*) e tornar a leitura muito mais intuitiva.

---

## A Estrutura de uma Função

Toda função em C/C++ (linguagem base do Arduino) segue uma anatomia padrão:

```cpp
tipoDeRetorno nomeDaFuncao(tipoParametro1 parametro1, tipoParametro2 parametro2) {
    // Corpo da função (código que será executado)
    return valor; // Opcional (apenas se houver tipo de retorno)
}
```

Para entender como utilizá-las na prática, vamos dividi-las em 4 categorias baseadas na presença de **parâmetros de entrada** (dados que a função recebe) e **retornos** (dados que a função envia de volta).

## 1. Funções Sem Parâmetros e Sem Retorno

É o tipo mais simples de função. Ela serve apenas para agrupar um conjunto de instruções que você deseja executar repetidamente, sem a necessidade de enviar ou receber dados de fora.

* **Tipo de retorno:** Usamos a palavra-chave `void` (vazio), indicando que ela não devolve nada.

* **Parâmetros:** Os parênteses ficam vazios `()`.
  
  ### Exemplo Prático:
  
  Imagine que queremos criar um padrão de alerta visual piscando um LED de forma específica.
  
  ```cpp
  const int ledPin = D4; // LED embutido no Wemos D1 Mini
  
  void setup() {
      pinMode(ledPin, OUTPUT);
  }
  
  void loop() {
      // Chamamos a nossa função personalizada
      emitirAlertaSOS(); 
      
      delay(3000); // Aguarda 3 segundos antes de repetir o Alerta
  }
  
  // Definição da nossa função
  void emitirAlertaSOS() {
      // Três piscadas rápidas
      for(int i = 0; i < 3; i++) {
        digitalWrite(ledPin, LOW);  // Liga (Active-LOW no Wemos)
        delay(200);
        digitalWrite(ledPin, HIGH); // Desliga
        delay(200);
      }
  }
  ```

---

## 2. Funções Com Parâmetros e Sem Retorno

Muitas vezes, queremos que a função execute a mesma tarefa, mas com dados dinâmicos. Os parâmetros de entrada funcionam como variáveis locais que recebem valores no momento em que a função é chamada.

* **Tipo de retorno:** Continua sendo `void`.

* **Parâmetros:** Especificamos o tipo e o nome de cada variável dentro dos parênteses.
  
  ### Exemplo Prático:
  
  Em vez de piscar sempre no mesmo ritmo estático, vamos passar o número de piscadas e o tempo de duração por parâmetro.
  
  ```cpp
  const int ledPin = D4;
  
  void setup() {
      pinMode(ledPin, OUTPUT);
  }
  
  void loop() {
      // Pisca o LED 5 vezes com intervalo de 100ms (Rápido)
      piscarLed(5, 100);
      delay(2000);
      
      // Pisca o LED 2 vezes com intervalo de 1000ms (Lento)
      piscarLed(2, 1000);
      delay(2000);
  }
  
  // Função que aceita dois parâmetros inteiros
  void piscarLed(int quantidade, int tempoEspera) {
      for(int i = 0; i < quantidade; i++) {
        digitalWrite(ledPin, LOW);
        delay(tempoEspera);
        digitalWrite(ledPin, HIGH);
        delay(tempoEspera);
      }
  }
  ```

---

## 3. Funções Sem Parâmetros e Com Retorno

Essas funções realizam um processamento interno (como ler um sensor ou calcular uma constante fixa) e enviam o resultado de volta para o bloco de código que a chamou.

* **Tipo de retorno:** Substituímos o `void` pelo tipo de dado que será devolvido (ex: `int`, `float`, `char`, `bool`).

* **Corpo da função:** É obrigatório o uso da palavra-chave `return` seguida do valor ou variável.
  
  ### Exemplo Prático:
  
  Simulação da leitura de um sensor analógico (como um LDR de luminosidade conectado ao pino A0) que converte a leitura diretamente em uma escala percentual.
  
  ```cpp
  void setup() {
      Serial.begin(9600);
  }
  
  void loop() {
      // A variável 'luminosidade' recebe o valor devolvido pela função
      int luminosidade = lerLuminosidadePercentual();
      
      Serial.print("Luminosidade atual: ");
      Serial.print(luminosidade);
      Serial.println("%");
      
      delay(1000);
  }
  
  // Função que retorna um número inteiro
  int lerLuminosidadePercentual() {
      int leituraBruta = analogRead(A0); // Lê um valor entre 0 e 1024
      
      // Mapeia o valor bruto para uma escala de 0 a 100
      int percentual = map(leituraBruta, 0, 1023, 0, 100); 
      
      return percentual; // Envia o dado processado de volta
  }
  ```

---

## 4. Funções Com Parâmetros e Com Retorno

Este é o nível mais completo e poderoso de uma função. Ela recebe um ou mais dados de entrada, realiza operações matemáticas ou lógicas complexas e devolve um resultado limpo. Elas funcionam exatamente como as funções matemáticas clássicas: $f(x, y) = resultado$.

### Exemplo Prático:

Uma função utilitária para converter uma temperatura de Celsius (comum no Brasil) para Fahrenheit.

```cpp
void setup() {
  Serial.begin(9600);
}
void loop() {
  float tempCelsius = 25.5;

  // Passamos o valor float e guardamos o retorno float
  float tempFahrenheit = converterCelsiusParaFahrenheit(tempCelsius);

  Serial.print(tempCelsius);
  Serial.print(" C equivale a ");
  Serial.print(tempFahrenheit);
  Serial.println(" F");

  delay(5000);
}

// Recebe um float e retorna um float
float converterCelsiusParaFahrenheit(float celsius) {
  float fahrenheit = (celsius * 9.0 / 5.0) + 32.0;
  return fahrenheit;
}
```

---

## Boas Práticas ao Criar Funções

1. **Nomes descritivos:** Use verbos no infinitivo ou termos que deixem claro o que a função faz (ex: `calcularMedia()`, `acionarMotores()`, e não nomes vagos como `funcao1()`, `processo()`).
2. **Responsabilidade Única:** Uma função deve fazer apenas **uma** coisa bem feita. Se uma função chamada `lerBotao()` também acende um LED e envia dados via Wi-Fi, divida-a em três funções separadas.
3. **Escopo de Variáveis:** Variáveis criadas dentro de uma função existem apenas dentro dela (variáveis locais). Elas são destruídas assim que a função termina a execução.

---

## 🚀 Desafio Prático para os Alunos

Para consolidar o conhecimento desta aula de laboratório, modifique o circuito do Semáforo Simples ou utilize a sua protoboard para resolver o seguinte problema de automação:

### O Sistema de Resfriamento Inteligente

Você deve criar um programa que monitore a temperatura de uma máquina de laboratório e acione um LED de alerta (simulando um cooler ou alarme de segurança).

1. Escreva uma função com retorno chamada `obterTemperatura()` que simule ou leia um sensor de temperatura e retorne o valor em graus Celsius (`float`).
2. Escreva uma função chamada `controlarAlerta()` que receba a temperatura por parâmetro (`float`), não retorne nada (`void`), e execute a seguinte lógica:
   * Se a temperatura for **menor que 30.0°C**, o LED deve ficar **desligado**.
   * Se a temperatura estiver **entre 30.0°C e 40.0°C**, o LED deve piscar **lentamente** (intervalo de 1 segundo).
   * Se a temperatura for **maior que 40.0°C**, o LED deve piscar **rapidamente** (intervalo de 200ms), indicando estado crítico de superaquecimento.
3. No seu `loop()`, faça a chamada ordenada dessas duas funções estruturadas de forma limpa.
