// Adafruit_NeoMatrix example for single NeoPixel Shield.
// Scrolls 'Howdy' across the matrix in a portrait (vertical) orientation.

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif
#include <Timer.h>

#define MATRIX_PIN 6
#define SONAR_PIN 5
#define SPEAKER_PIN 10
#define LRS_TRANSITION 10
#define SRS_TRANSITION 10
#define AD_TRANSITION 15

enum State {
  LONG_RANGE_SCAN,
  SHORT_RANGE_SCAN,
  AWESOME_DETECTED,
  CLEAR
};

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 8, MATRIX_PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);


uint16_t w    = matrix.width();
uint16_t h    = matrix.height();
State current_state;
boolean detect;
uint16_t range;
Timer t;
int scan_event;
int16_t upgrade_transition_count;
int16_t downgrade_transition_count;
uint16_t state_delay;
//holds the "walkers" for the long range scanner.
int16_t lrs_walkers[SRS_TRANSITION-1][6]; 


void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0)-analogRead(1)+analogRead(2)-analogRead(3)+analogRead(4));
  pinMode(SONAR_PIN,INPUT);
  pinMode(SPEAKER_PIN,OUTPUT);
  matrix.begin();
  matrix.setBrightness(10);
  matrix.setTextWrap(false);
  matrix.setTextColor(matrix.Color(255,0,0));
  matrix.fillScreen(0);
  matrix.show();
  delay(500); //allow sonar to calibrate and get first reading
  for (uint16_t i; i<SRS_TRANSITION-1;i++) {
    lrs_walkers[i][0]=random(4);
    lrs_walkers[i][1]=random(8);
    lrs_walkers[i][2]=random(8);
    lrs_walkers[i][3]=random(1,255);    
    lrs_walkers[i][4]=random(1,255); 
    lrs_walkers[i][5]=random(1,255);    
  }
}

void loop() {
/*
  for (int i=255;i>=0;i--) {
      playTone(255-(i/2),1);   
  }
*/

  int x = random(w);
  int y = random(h);
  int r = random(1,w);
  
  matrix.drawCircle(x,y,r,matrix.Color(random(255), random(255), random(255)));
  matrix.show();
  matrix.drawCircle(x,y,r,0);
  delay(200);

  t.update();
/*
  int lit_rows;   // how many rows of the view we'll light up  
                  
    matrix.fillScreen(0);
    lit_rows=9-(modE/10);
    Serial.print(pulse);Serial.print(" ");Serial.print(modE);Serial.print(" ");Serial.println(lit_rows);
    for (int i=lit_rows-1;i>=0;--i) {
      lightRow(i);
    }
    matrix.show();

  delay(60); // wait between samples
*/

/*
  playTone(125,150);
  delay(1000);
  playTone(125,300);
  delay(1000);
  playTone(125,150);
  delay(5000);
*/
}

void startLongRangeScan() {
  Serial.println("Switching to long range scanner...");
  t.stop(scan_event);
  detect=false;
  matrix.fillScreen(0);
  matrix.show();
  current_state=LONG_RANGE_SCAN;
  upgrade_transition_count=0;  
  scan_event=t.every(1000,getRange);
  Countdown();
}

void startShortRangeScan() {
  Serial.println("Switching to short range scanner...");
  t.stop(scan_event);
  detect=false;
  matrix.fillScreen(0);
  for (int i=255;i>=50;i--) {
    matrix.drawRect(0,0,w,h,matrix.Color(i,i,i));
    matrix.show();
    playTone(255-(i/2)-25,1); 
  }
  current_state=SHORT_RANGE_SCAN;
  downgrade_transition_count=0; 
  upgrade_transition_count=0;
  scan_event=t.every(250,getRange);
}

void AwesomeDetected() {
  Serial.println("Awesome detected!");
  t.stop(scan_event);
  detect=false;
  matrix.fillScreen(0);
  matrix.show();
  current_state=AWESOME_DETECTED;
  scan_event=t.after(5000L,Clear);
}

void Clear() {
  Serial.println("Clear...");
  detect=false;
  matrix.fillScreen(0);
  matrix.show();
  current_state=CLEAR;
  scan_event=t.after(15000L,startLongRangeScan);  
}

void getRange() {
    range=pulseIn(SONAR_PIN, HIGH)/147;
    detect=true;
}

void lightRow(int row) {
  for (int i=0; i<w; ++i) {
    matrix.drawPixel(i,row,matrix.Color(255, 0, 0));   
  }
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(SPEAKER_PIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(SPEAKER_PIN, LOW);
    delayMicroseconds(tone);
  }
}

void correctForWall(uint16_t walker, uint16_t dest_index, uint16_t bound) {
/*
  Serial.print("++++Correct ");Serial.print(walker);Serial.print(" ");
  Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/
  if (lrs_walkers[walker][dest_index]<0 || lrs_walkers[walker][dest_index] >= bound) {
       lrs_walkers[walker][dest_index]=max(0,lrs_walkers[walker][dest_index]);
       lrs_walkers[walker][dest_index]=min(lrs_walkers[walker][dest_index],bound-1);
       lrs_walkers[walker][2]=random(8);
/*        
        Serial.print("++++Fixing impact ");Serial.print(walker);Serial.print(" ");
        Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
        Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
        Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
        Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
        Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
        Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/
       moveWalker(walker);
  }
}

void moveUp(uint16_t walker) {
/*
  Serial.print("++Up ");Serial.print(walker);Serial.print(" ");
  Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/
  lrs_walkers[walker][1] -=1;
  correctForWall(walker,1,h);
/*
  Serial.print("-- ");Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/
}

void moveDown(uint16_t walker) {
/*
  Serial.print("++Down ");Serial.print(walker);Serial.print(" ");
  Serial.print(lrs_walkers[walker][0]);Serial.print(" ");  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/
  lrs_walkers[walker][1] +=1;
  correctForWall(walker,1,h); 
/*
  Serial.print("-- ");Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/ 
}

void moveLeft(uint16_t walker) {
/*     
  Serial.print("++Left ");Serial.print(walker);Serial.print(" ");
  Serial.print(lrs_walkers[walker][0]);Serial.print(" ");  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/
  lrs_walkers[walker][0] -=1;
  correctForWall(walker,0,w);
/* 
  Serial.print("-- ");Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/ 
}

void moveRight(uint16_t walker) {
/*
  Serial.print("++Right ");Serial.print(walker);Serial.print(" ");
  Serial.print(lrs_walkers[walker][0]);Serial.print(" ");  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/
  lrs_walkers[walker][0] +=1;
  correctForWall(walker,0,w); 
/*
  Serial.print("-- ");Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
*/ 
}

void moveWalker(uint16_t walker) {
  switch(lrs_walkers[walker][2]) {
    case 0: //up, left
            moveUp(walker);
            moveLeft(walker);
            break;
    case 1: //up
            moveUp(walker);
            break;
    case 2: //up, right
            moveUp(walker);
            moveRight(walker);
            break;
    case 3: //right
            moveRight(walker);
            break;
    case 4: //down, right
            moveDown(walker);
            moveRight(walker);
            break;
    case 5: //down
            moveDown(walker);
            break;
    case 6: //down, left
            moveDown(walker);
            moveLeft(walker);
            break;
    case 7: //left
            moveLeft(walker);
            break;           
  }
}

void Countdown() {
      matrix.fillScreen(matrix.Color(255,0,0));
      matrix.show();
      for (int i=0;i<h;i+=2) {  
        playTone(75,150);
        delay(850);
        matrix.drawRect(0,i,w,2,0);
        matrix.show();
      }
      for (int i=1;i<=3;i++) {
        playTone(75,50); 
        delay(50);       
      }
}

void WipeOut(uint16_t color,int drag) {
      for (int i=1;i<=h/2;i++) {
        matrix.fillRect((i<3)?((w/2)-1):(0),(h/2)-i,(i<3)?(2):(w),i*2,color);     
        matrix.show();
        playTone(175-(20*i),drag);
      }
}


