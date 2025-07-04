/*
 * Open Decabot LED Matrix example
 * Daniel Chagas - https://github.com/triwaca/openDecabot
 * Hardware:
 * - Wemos D1 Mini ESP8266 board: https://www.wemos.cc/en/latest/d1/d1_mini.html
 * - LED Matrix Wemos mini shield: https://www.wemos.cc/en/latest/d1_mini_shield/matrix_led.html
 * 
 * EN: Will exibit Decabot eyes on Matrix display, blinking and looking sideways.
 * PT: Irá mostrar os olhos do Decabot no display de matriz, piscando e olhando para os lados.
 * 
 */
#include <Adafruit_GFX.h>       //tested on version 1.11.10
#include <WEMOS_Matrix_GFX.h>   //tested on version 1.4.0

MLED matrix(7); //set intensity=7 (maximum)

int lpos = 1; //left eye position
int rpos = 6; //right eye position
int aleat = 5; //aleatory position

void setup() {
  //flip the matrix upside down
  matrix.setRotation(2);
}

void loop() {
  matrix.clear();
  matrix.drawLine(lpos,2, lpos,7, LED_ON);
  matrix.drawLine(rpos,2, rpos,7, LED_ON);
  matrix.writeDisplay();
  delay(1000);
  //create the blink effect
  for(int j=0;j<random(3);j++){
    for(int i=2;i<7;i++){
      matrix.clear();
      matrix.drawLine(lpos,i, lpos,7, LED_ON);
      matrix.drawLine(rpos,i, rpos,7, LED_ON);
      matrix.writeDisplay();
      delay(15);
    }
    for(int i=7;i>2;i--){
      matrix.clear();
      matrix.drawLine(lpos,i, lpos,7, LED_ON);
      matrix.drawLine(rpos,i, rpos,7, LED_ON);
      matrix.writeDisplay();
      delay(15);
    }  
  }
  //create the look sideways effect
  aleat = random(10);
  if(aleat<2){
    lpos = 0;
    rpos = 5;
  } else if(aleat>7){
    lpos = 2;
    rpos = 7;
  } else {
    lpos = 1;
    rpos = 6;
  }
}
