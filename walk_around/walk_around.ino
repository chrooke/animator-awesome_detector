// Adafruit_NeoMatrix example for single NeoPixel Shield.
// Scrolls 'Howdy' across the matrix in a portrait (vertical) orientation.

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


// Example for NeoPixel Shield.  In this application we'd like to use it
// as a 4x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };


int w    = matrix.width();
int h    = matrix.height();
int rx   = random(w);
int ry   = random(h);
int rr,rg,rb;
int direction = random(8);
#define DELAY 75

void correctForWall(int *dest, int bound) {
  Serial.print(rx);Serial.print(" ");
  Serial.print(ry);Serial.print(" ");
  Serial.print(w);Serial.print(" ");
  Serial.print(h);Serial.print(" ");
  Serial.print(*dest);Serial.print(" ");
  Serial.println(bound);Serial.print(" ");
  if (*dest<0 || *dest >= bound) {
       *dest=max(0,*dest);
       *dest=min(*dest,bound-1);
       direction=random(8);
       movePixel(direction);
  }
}

void moveUp() {
  ry -=1;
  correctForWall(&ry,h);
}

void moveDown() {
  ry +=1;
  correctForWall(&ry,h);  
}

void moveLeft() {
  rx -=1;
  correctForWall(&rx,w);  
}

void moveRight() {
  rx +=1;
  correctForWall(&rx,w);  
}

void movePixel(int direction) {
  switch(direction) {
    case 0: //up, left
            moveUp();
            moveLeft();
            break;
    case 1: //up
            moveUp();
            break;
    case 2: //up, right
            moveUp();
            moveRight();
            break;
    case 3: //right
            moveRight();
            break;
    case 4: //down, right
            moveDown();
            moveRight();
            break;
    case 5: //down
            moveDown();
            break;
    case 6: //down, left
            moveDown();
            moveLeft();
            break;
    case 7: //left
            moveLeft();
            break;           
  }
}
void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.setBrightness(10);
}


void loop() {
  rr = random(128,255);
  rg = random(128,255);
  rb = random(128,255);
  
  matrix.drawPixel(rx,ry,matrix.Color(rr, rg, rb));
  matrix.show();
  delay(DELAY);
  matrix.drawPixel(rx,ry,0);
  matrix.show();
  movePixel(direction);
  
}
