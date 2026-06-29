# Piscar um LED (Blink) com o Wemos D1 Mini

Bem-vindos a este guia de introdução prática. Este tutorial foi desenvolvido para capacitar os alunos nas atividades da disciplina de Experimentação Orientada, servindo como base para projetos de robótica open-source, como o *openDecabot*. O objetivo é compreender a estrutura de programação, a pinagem do microcontrolador e a lógica de controle de hardware através da plataforma Wemos D1 Mini, baseada no chip ESP8266.

## 1. Objetivos da Aula

* Configurar o ambiente de desenvolvimento Arduino IDE para o Wemos D1 Mini.
* Compreender o mapeamento de pinos (Pinout) da placa.
* Escrever e carregar um código para controlar o LED embutido (built-in LED).
  
  ## 2. Conhecendo o Hardware: Wemos D1 Mini
  
  O Wemos D1 Mini é uma placa de desenvolvimento compacta com conectividade Wi-Fi nativa, excelente para prototipagem de sistemas embarcados. Um detalhe crucial para iniciantes é o mapeamento dos pinos (GPIO). A numeração impressa na placa (como D1, D2, D4) não corresponde diretamente ao número do GPIO lido internamente pelo chip.
  Neste exercício, utilizaremos o LED azul que já vem soldado na placa. Veja a relação de pinos na tabela abaixo:
  
  | Pino na Placa (Label)                                                                                                                                            | GPIO Interno (ESP8266) | Função e Observações                                                    |
  |:---------------------------------------------------------------------------------------------------------------------------------------------------------------- |:---------------------- |:----------------------------------------------------------------------- |
  | **D4**                                                                                                                                                           | GPIO 2                 | Conectado ao **LED Embutido**. Opera com lógica invertida (Active-LOW). |
  | **Importante:** O LED do Wemos D1 Mini opera em *Active-LOW*. Isso significa que enviar um sinal numérico 0 (LOW) liga o LED, e um sinal 1 (HIGH) desliga o LED. |                        |                                                                         |
  
  ## 3. Materiais Necessários
* 1x Placa Wemos D1 Mini (ESP8266)
* 1x Cabo Micro-USB para transferência de dados
* Computador com a Arduino IDE instalada
  
  ## 4. Preparando a Arduino IDE
1. Abra a Arduino IDE, vá em **Arquivo > Preferências**.

2. No campo de URLs Adicionais para Gerenciadores de Placas, insira o link oficial do pacote ESP8266 e clique em OK.

3. Vá em **Ferramentas > Placa > Gerenciador de Placas...**. Busque por "esp8266" e instale o pacote desenvolvido pela comunidade ESP8266.

4. Após a instalação, vá em **Ferramentas > Placa** e selecione **LOLIN(WEMOS) D1 R2 & mini**.

5. Conecte sua placa ao computador usando o cabo USB e selecione a porta correta em **Ferramentas > Porta**.
   
   ## 5. O Código-Fonte (Sketch)
   
   Copie o código abaixo para a sua IDE. Leia os comentários atentamente para entender o fluxo de execução do programa.
   
   ```cpp
   /*
   Laboratório Lapin - Introdução a Microcontroladores
   Projeto: Blink (Piscar LED) com Wemos D1 Mini
   */
   // O pino D4 (GPIO2) possui o LED azul embutido na placa
   const int ledPin = D4; 
   void setup() {
       // Configura o pino do LED como uma saída (OUTPUT)
       pinMode(ledPin, OUTPUT);
   }
   void loop() {
       // Liga o LED (lembre-se: Active-LOW, então LOW liga o LED)
       digitalWrite(ledPin, LOW);  
   
       // Aguarda 1000 milissegundos (1 segundo)
       delay(1000);                      
   
       // Desliga o LED (HIGH desliga)
       digitalWrite(ledPin, HIGH); 
   
       // Aguarda mais 1 segundo antes de reiniciar o ciclo
       delay(1000);                      
   }
   ```
   
   ## 6. Compilando e Carregando
* Clique no botão **Verificar** (ícone de V) no canto superior esquerdo para compilar o código e verificar se há erros de sintaxe.
* Clique no botão **Carregar** (ícone de Seta para a direita). A IDE irá compilar e transferir o firmware para a memória flash do Wemos D1 Mini.
* Assim que aparecer a mensagem de transferência concluída na parte inferior da tela, o LED azul da sua placa começará a piscar em intervalos de 1 segundo!
  
  ## 7. Desafio Prático
  
  Agora que o código base está funcionando, tente modificar a frequência das piscadas. Experimente fazer o LED piscar rapidamente como um estrobo (exemplo: 100ms ligado, 100ms desligado), ou criar um padrão assimétrico alternando entre tempos longos e curtos na função `delay()`. Esses testes são essenciais para dominar a lógica de tempo em sistemas embarcados antes de avançarmos para o controle de motores e leitura de sensores.


