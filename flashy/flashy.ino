#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };

void setup() {
  matrix.begin();
  matrix.setBrightness(10);
}

int w    = matrix.width();
int h    = matrix.height();
int rx,ry,rr,rg,rb;

void loop() {
  rx = random(w);
  ry = random(h);
  rr = random(255);
  rg = random(255);
  rb = random(255);
  matrix.drawPixel(rx,ry,matrix.Color(rr, rg, rb));
  matrix.show();
}
