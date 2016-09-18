//Comment out to make the program silent
#define AUDIO

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
#define NUM_STATES 7
#define TIME_PER_STATE 30000L

enum State {
  SINGLE_WALKER,
  WALKERS,
  FLASHY,
  SINGLE_HIVE,
  HIVES,
  CIRCLES,
  LINES
};

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 8, MATRIX_PIN,
  NEO_MATRIX_BOTTOM  + NEO_MATRIX_LEFT +
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
uint8_t num_walkers=0; 
uint8_t x;
uint8_t y;
uint8_t r;


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
    case SINGLE_WALKER:
        //move walkers
        matrix.fillScreen(0);
        for (uint16_t i=0;i<num_walkers;i++){
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

    case HIVES:
    case SINGLE_HIVE:
        //move hive
        matrix.fillScreen(0);
        for (uint16_t i=0;i<num_walkers;i++) {
          matrix.drawPixel(lrs_walkers[i][0],
                           lrs_walkers[i][1],
                           matrix.Color(
                           lrs_walkers[i][3],
                           lrs_walkers[i][4],
                           lrs_walkers[i][5]));
          moveWalker(i);
          lrs_walkers[i][2]=random(8);         
        }
        matrix.show();
        break;

    case CIRCLES:
        x = random(w);
        y = random(h);
        r = random(1,w);
        matrix.drawCircle(x,y,r,matrix.Color(random(255), random(255), random(255)));
        matrix.show();
        matrix.drawCircle(x,y,r,0);
        break;

    case LINES:
        x = random(w);
        y = random(w);
        matrix.drawLine(x,0,y,h,matrix.Color(random(255), random(255), random(255)));
        matrix.show();
        matrix.drawLine(x,0,y,h,0);
        break;
  }

  delay(state_delay);
  t.update();
}

void changeState() {
  randomTransition();
#ifdef DEBUG
      Serial.print("Switching to ");
#endif
  switch (random(NUM_STATES)) {
    case SINGLE_WALKER:
#ifdef DEBUG
      Serial.println("SINGLE_WALKER...");
#endif
      current_state=SINGLE_WALKER;
      num_walkers=1; 
      state_delay=random(50);
      initializeWalkers();  
      break;
      
    case WALKERS:
#ifdef DEBUG
      Serial.println("WALKERS...");
#endif
      current_state=WALKERS;
      state_delay=random(100,750);
      num_walkers=random(1,MAX_WALKERS);
      initializeWalkers();
      break;
      
    case FLASHY:
#ifdef DEBUG
  Serial.println("FLASHY...");
#endif
      current_state=FLASHY;
      state_delay=random(250);
      break;
      
    case SINGLE_HIVE:
#ifdef DEBUG
      Serial.println("SINGLE_HIVE...");
#endif
      current_state=SINGLE_HIVE;
      num_walkers=1;
      state_delay=20;
      initializeWalkers();
      break;
          
    case HIVES:
#ifdef DEBUG
      Serial.println("HIVES...");
#endif
      current_state=HIVES;
      num_walkers=random(1,4);
      state_delay=20;
      initializeWalkers();
      break;

    case CIRCLES:
#ifdef DEBUG
      Serial.println("CIRCLES...");
#endif
      current_state=CIRCLES;
      state_delay=random(1,11)*10;
      break;

#ifdef DEBUG
      Serial.print("   num_walkers: ");Serial.println(num_walkers);
      Serial.print("   state_delay: ");Serial.println(state_delay);
#endif

    case LINES:
#ifdef DEBUG
      Serial.println("LINES...");
#endif
      current_state=LINES;
      state_delay=random(50,150);
      break;
  }
#ifdef DEBUG
      Serial.print("   num_walkers: ");Serial.println(num_walkers);
      Serial.print("   state_delay: ");Serial.println(state_delay);
#endif
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(SPEAKER_PIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(SPEAKER_PIN, LOW);
    delayMicroseconds(tone);
  }
}

void initializeWalkers() {
  for (uint16_t i; i<num_walkers;i++) {
    lrs_walkers[i][0]=random(w);
    lrs_walkers[i][1]=random(h);
    lrs_walkers[i][2]=random(8);
    lrs_walkers[i][3]=random(1,255);    
    lrs_walkers[i][4]=random(1,255); 
    lrs_walkers[i][5]=random(1,255);    
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
#ifdef DEBUG
      Serial.print("Transition: ");
#endif  
  switch(random(10)) {
    case 0:
#ifdef DEBUG
      Serial.println("Red Countdown");
#endif
      Countdown(matrix.Color(255,0,0));
      break;
    case 1:
#ifdef DEBUG
      Serial.println("Green Countdown");
#endif
      Countdown(matrix.Color(0,255,0));
      break;
    case 2:
#ifdef DEBUG
      Serial.println("Blue Countdown");
#endif
      Countdown(matrix.Color(0,0,255));
      break;
    case 3:
#ifdef DEBUG
      Serial.println("Random Countdown");
#endif
      Countdown(matrix.Color(random(255),random(255),random(255)));
      break;
    case 4:
#ifdef DEBUG
      Serial.println("Red Wipeout");
#endif
      WipeOut(matrix.Color(255,0,0),100);  
      WipeOut(0,100);
      break;
    case 5:
#ifdef DEBUG
      Serial.println("Green Wipeout");
#endif
      WipeOut(matrix.Color(0,255,0),100);  
      WipeOut(0,100);
      break;
    case 6:
#ifdef DEBUG
      Serial.println("Blue Wipeout");
#endif
      WipeOut(matrix.Color(0,0,255),100);  
      WipeOut(0,100);
      break;
    case 7:
#ifdef DEBUG
      Serial.println("Random Wipeout");
#endif
      WipeOut(matrix.Color(random(255),random(255),random(255)),100);  
      WipeOut(0,100);
      break;
    case 8:
#ifdef DEBUG
      Serial.println("Random Double Wipeout");
#endif
      WipeOut(matrix.Color(random(255),random(255),random(255)),100);
      WipeOut(matrix.Color(random(255),random(255),random(255)),75);  
      WipeOut(0,50); 
      break;
    case 9:
#ifdef DEBUG
      Serial.println("Framer");
#endif
      Framer();
      break;     
  }
}

void Countdown(uint16_t color) {
      matrix.fillScreen(color);
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

