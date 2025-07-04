
#include <Adafruit_GFX.h>
#include <WEMOS_Matrix_GFX.h>

MLED matrix(7); //set intensity=7 (maximum)

int lpos = 1;
int rpos = 6;
int aleat = 5;

void setup() {
  matrix.setRotation(2);
}

void loop() {
  matrix.clear();
  //matrix.setCursor(0,0);
  //matrix.print("AB");
  matrix.drawLine(lpos,2, lpos,7, LED_ON);
  matrix.drawLine(rpos,2, rpos,7, LED_ON);
  matrix.writeDisplay();
  delay(1000);
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
