//Comment out to make the program silent
//#define AUDIO

//Remove comment to get debug information to Serial
//#define DEBUG 

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif
#include <Timer.h>

#define MATRIX_PIN 6
#define SPEAKER_PIN 10
#define MAX_WALKERS 10
#define NUM_STATES 3
#define TIME_PER_STATE 30000L

enum State {
  WALKERS,
  FLASHY,
  HIVE
};

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 8, MATRIX_PIN,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);


uint16_t w    = matrix.width();
uint16_t h    = matrix.height();
State current_state;
Timer t;
uint8_t scan_event;
uint16_t state_delay;
//holds the "walkers" for the long range scanner.
int16_t lrs_walkers[MAX_WALKERS][6];
uint8_t num_walkers; 


void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  randomSeed(analogRead(0)-analogRead(1)+analogRead(2)-analogRead(3)+analogRead(4));
  pinMode(SPEAKER_PIN,OUTPUT);
  matrix.begin();
  matrix.setBrightness(10);
  scan_event=t.every(TIME_PER_STATE,changeState);
  changeState();
}

void loop() {
  switch (current_state) {
    case WALKERS:
        //move walkers
        matrix.fillScreen(0);
        for (uint16_t i=0;i<=num_walkers;i++){
          matrix.drawPixel(lrs_walkers[i][0],
                           lrs_walkers[i][1],
                           matrix.Color(
                           lrs_walkers[i][3],
                           lrs_walkers[i][4],
                           lrs_walkers[i][5]));
          moveWalker(i);         
        }
        matrix.show();
        break;
    
    case FLASHY:
        matrix.drawPixel(random(w),random(h),matrix.Color(random(255), random(255), random(255)));
        matrix.show();
        break;

    case HIVE:
        //move hive
        matrix.fillScreen(0);
        matrix.drawPixel(lrs_walkers[0][0],
                         lrs_walkers[0][1],
                         matrix.Color(
                         lrs_walkers[0][3],
                         lrs_walkers[0][4],
                         lrs_walkers[0][5]));
        moveWalker(0);
        lrs_walkers[0][2]=random(8);         
        matrix.show();
        break;
  }

  delay(state_delay);
  t.update();
}

void startWalkers() {
#ifdef DEBUG
  Serial.println("Switching to walkers...");
#endif
  current_state=WALKERS;
  num_walkers=random(MAX_WALKERS);
  state_delay=random(100,750);
  for (uint16_t i; i<MAX_WALKERS;i++) {
    lrs_walkers[i][0]=random(w);
    lrs_walkers[i][1]=random(h);
    lrs_walkers[i][2]=random(8);
    lrs_walkers[i][3]=random(1,255);    
    lrs_walkers[i][4]=random(1,255); 
    lrs_walkers[i][5]=random(1,255);    
  }  
  randomTransition();
}

void startFlashy() {
#ifdef DEBUG
  Serial.println("Switching to flashy...");
#endif
  current_state=FLASHY;
  state_delay=random(250);
  randomTransition();
}

void startHive() {
#ifdef DEBUG
  Serial.println("Switching to hive...");
#endif
  current_state=HIVE;
  num_walkers=1;
  state_delay=20;
  lrs_walkers[0][0]=random(w);
  lrs_walkers[0][1]=random(h);
  lrs_walkers[0][2]=random(8);
  lrs_walkers[0][3]=random(1,255);    
  lrs_walkers[0][4]=random(1,255); 
  lrs_walkers[0][5]=random(1,255);    
  randomTransition();
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(SPEAKER_PIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(SPEAKER_PIN, LOW);
    delayMicroseconds(tone);
  }
}

void changeState() {
  switch (random(NUM_STATES)) {
    case WALKERS:
      startWalkers();
      break;
    case FLASHY:
      startFlashy();
      break;
    case HIVE:
      startHive();
  }
}

void moveWalker(uint16_t walker) {
  int16_t cx,cy;
  switch(lrs_walkers[walker][2]) {
    case 0: //up, left
            cy=lrs_walkers[walker][1]-1;
            cx=lrs_walkers[walker][0]-1;
            break;
    case 1: //up
            cy=lrs_walkers[walker][1]-1;
            cx=lrs_walkers[walker][0];
            break;
    case 2: //up, right
            cy=lrs_walkers[walker][1]-1;
            cx=lrs_walkers[walker][0]+1;
            break;
    case 3: //right
            cy=lrs_walkers[walker][1];
            cx=lrs_walkers[walker][0]+1;
            break;
    case 4: //down, right
            cy=lrs_walkers[walker][1]+1;
            cx=lrs_walkers[walker][0]+1;
            break;
    case 5: //down
            cy=lrs_walkers[walker][1]+1;
            cx=lrs_walkers[walker][0];
            break;
    case 6: //down, left
            cy=lrs_walkers[walker][1]+1;
            cx=lrs_walkers[walker][0]-1;
            break;
    case 7: //left
            cy=lrs_walkers[walker][1];
            cx=lrs_walkers[walker][0]-1;
            break;           
  }
  if (cx<0 || cx >= w || cy<0 || cy >= h) { //collision! Try a different direction.
    lrs_walkers[walker][2]=random(8);
    moveWalker(walker);
  } else { //OK
    lrs_walkers[walker][0]=cx;
    lrs_walkers[walker][1]=cy;
  }
}

void randomTransition() {
  switch(random(6)) {
    case 0:
#ifdef DEBUG
      Serial.println("Transistion: Countdown");
#endif
      Countdown();
      break;
    case 1:
#ifdef DEBUG
      Serial.println("Transistion: Red Wipeout");
#endif
      WipeOut(matrix.Color(255,0,0),100);  
      WipeOut(0,100);
      break;
    case 2:
#ifdef DEBUG
      Serial.println("Transistion: Green Wipeout");
#endif
      WipeOut(matrix.Color(0,255,0),100);  
      WipeOut(0,100);
      break;
    case 3:
#ifdef DEBUG
      Serial.println("Transistion: Blue Wipeout");
#endif
      WipeOut(matrix.Color(0,0,255),100);  
      WipeOut(0,100);
      break;
    case 4:
#ifdef DEBUG
      Serial.println("Transistion: Random Double Wipeout");
#endif
      WipeOut(matrix.Color(random(255),random(255),random(255)),100);
      WipeOut(matrix.Color(random(255),random(255),random(255)),75);  
      WipeOut(0,50); 
      break;
    case 5:
#ifdef DEBUG
      Serial.println("Transistion: Framer");
#endif
      Framer();
      break;     
  }
}

void Countdown() {
      matrix.fillScreen(matrix.Color(255,0,0));
      matrix.show();
      for (int i=0;i<h;i+=2) {  
  #ifdef AUDIO
        playTone(75,150);
        delay(850);
  #else
        delay(1000);
  #endif
        matrix.drawRect(0,i,w,2,0);
        matrix.show();
      }
  #ifdef AUDIO
      for (int i=1;i<=3;i++) {
        playTone(75,50); 
        delay(50);       
      }
  #endif
}

void WipeOut(uint16_t color,int drag) {
      for (int i=1;i<=h/2;i++) {
        matrix.fillRect((i<3)?((w/2)-1):(0),(h/2)-i,(i<3)?(2):(w),i*2,color);     
        matrix.show();
  #ifdef AUDIO
        playTone(175-(20*i),drag);
  #else
        delay(drag);
  #endif
      }
}

void Framer() {
  matrix.fillScreen(0);
  for (int i=255;i>=0;i--) {
    matrix.drawRect(0,0,w,h,matrix.Color(i,i,i));
    matrix.show();
  #ifdef AUDIO
    playTone(255-(i/2)-25,1);
  #endif
  }
}

