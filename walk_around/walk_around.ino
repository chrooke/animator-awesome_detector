#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define MATRIX_PIN 6

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 8, MATRIX_PIN,
  NEO_MATRIX_BOTTOM  + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);


uint16_t w    = matrix.width();
uint16_t h    = matrix.height();
int16_t walker[6]; 


void setup() {
  randomSeed(analogRead(0)-analogRead(1)+analogRead(2)-analogRead(3)+analogRead(4));
  matrix.begin();
  matrix.setBrightness(10);
  walker[0]=random(w);
  walker[1]=random(h);
  walker[2]=random(8);
  walker[3]=random(1,255);    
  walker[4]=random(1,255); 
  walker[5]=random(1,255); 
}

void loop() {
  matrix.fillScreen(0);
  matrix.drawPixel(walker[0],
                   walker[1],
                   matrix.Color(
                   walker[3],
                   walker[4],
                   walker[5]));
  moveWalker();
  walker[2]=random(8);         
  matrix.show();
 
  delay(random(50));
}


void moveWalker() {
  int16_t cx,cy;
  switch(walker[2]) {
    case 0: //up, left
            cy=walker[1]-1;
            cx=walker[0]-1;
            break;
    case 1: //up
            cy=walker[1]-1;
            cx=walker[0];
            break;
    case 2: //up, right
            cy=walker[1]-1;
            cx=walker[0]+1;
            break;
    case 3: //right
            cy=walker[1];
            cx=walker[0]+1;
            break;
    case 4: //down, right
            cy=walker[1]+1;
            cx=walker[0]+1;
            break;
    case 5: //down
            cy=walker[1]+1;
            cx=walker[0];
            break;
    case 6: //down, left
            cy=walker[1]+1;
            cx=walker[0]-1;
            break;
    case 7: //left
            cy=walker[1];
            cx=walker[0]-1;
            break;           
  }
  if (cx<0 || cx >= w || cy<0 || cy >= h) { //collision! Try a different direction.
    walker[2]=random(8);
    moveWalker();
  } else { //OK
    walker[0]=cx;
    walker[1]=cy;
  }
}

