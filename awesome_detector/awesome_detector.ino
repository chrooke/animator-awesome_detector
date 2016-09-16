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
uint8_t scan_event;
int16_t upgrade_transition_count;
int16_t downgrade_transition_count;
uint16_t state_delay;
//holds the "walkers" for the long range scanner.
int16_t lrs_walkers[SRS_TRANSITION-1][6]; 


void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  randomSeed(analogRead(0)-analogRead(1)+analogRead(2)-analogRead(3)+analogRead(4));
  pinMode(SONAR_PIN,INPUT);
  pinMode(SPEAKER_PIN,OUTPUT);
  matrix.begin();
  matrix.setBrightness(10);
  matrix.setTextWrap(false);
  matrix.setTextColor(matrix.Color(255,0,0));
  delay(500); //allow sonar to calibrate and get first reading
  for (uint16_t i; i<SRS_TRANSITION-1;i++) {
    lrs_walkers[i][0]=random(w);
    lrs_walkers[i][1]=random(h);
    lrs_walkers[i][2]=random(8);
    lrs_walkers[i][3]=random(1,255);    
    lrs_walkers[i][4]=random(1,255); 
    lrs_walkers[i][5]=random(1,255);    
  }
  startLongRangeScan();
}

void loop() {
  switch (current_state) {
    case LONG_RANGE_SCAN:
        if (detect) {
            if (range>90) {
              upgrade_transition_count=0;
            } else if (range>60) {
              upgrade_transition_count--;
              upgrade_transition_count=max(0,upgrade_transition_count);
            } else if (range>36) {
              upgrade_transition_count++;
            } else if (range>12) {
              upgrade_transition_count+=2;
            }
            if (range>12) {
#ifdef DEBUG
              Serial.print("Range: ");Serial.println(range);
              Serial.print("upgrade_transition_count: ");Serial.println(upgrade_transition_count);
#endif
            }
            detect=false;                  
         }
         if (upgrade_transition_count >= SRS_TRANSITION) {
            startShortRangeScan();
         }
        //move walkers
        matrix.fillScreen(0);
        for (uint16_t i=0;i<=upgrade_transition_count;i++){
          matrix.drawPixel(lrs_walkers[i][0],
                           lrs_walkers[i][1],
                           matrix.Color(
                           lrs_walkers[i][3],
                           lrs_walkers[i][4],
                           lrs_walkers[i][5]));
          moveWalker(i);         
        }
        matrix.show();
        state_delay=750-(700/SRS_TRANSITION)*upgrade_transition_count;
        delay(state_delay);
        break;
    
    case SHORT_RANGE_SCAN:
        if (detect) {
            if (range>40) {
              downgrade_transition_count++;
              upgrade_transition_count=0;
            } else if (range>30) {
              upgrade_transition_count--;
              upgrade_transition_count=max(0,upgrade_transition_count);
            } else if (range>20) {
              downgrade_transition_count--;
              downgrade_transition_count=max(0,downgrade_transition_count);
              upgrade_transition_count++;
            } else if (range>12) {
              downgrade_transition_count-=2;
              downgrade_transition_count=max(0,downgrade_transition_count);
              upgrade_transition_count++;
            }
            if (range>12) {
            }
            detect=false;
         }
        if (downgrade_transition_count >= LRS_TRANSITION) {
            startLongRangeScan();
        }
        if (upgrade_transition_count >= AD_TRANSITION) {
            AwesomeDetected();
        }
        matrix.drawPixel(random(w),random(h),matrix.Color(random(255), random(255), random(255)));
        matrix.drawPixel(random(w),random(h),matrix.Color(random(255), random(255), random(255)));
        matrix.show();
        state_delay=250-(240/AD_TRANSITION)*upgrade_transition_count;
        delay(state_delay);
      break;

    case AWESOME_DETECTED:
      matrix.fillScreen(0);
      matrix.setTextColor(matrix.Color(random(255), random(255), random(255)));
      matrix.setCursor(random(4)-2, 0);
      matrix.print(F("!"));
      matrix.show();
#ifdef AUDIO
      playTone(random(75,300),random(5,25));
#endif
      delay(100);
      break;

    case CLEAR:
      break;
  }

  t.update();
}

void startLongRangeScan() {
#ifdef DEBUG
  Serial.println("Switching to long range scanner...");
#endif
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
#ifdef DEBUG
  Serial.println("Switching to short range scanner...");
#endif
  t.stop(scan_event);
  detect=false;
  current_state=SHORT_RANGE_SCAN;
  downgrade_transition_count=0; 
  upgrade_transition_count=0;
  scan_event=t.every(250,getRange);
  matrix.fillScreen(0);
  for (int i=255;i>=0;i--) {
    matrix.drawRect(0,0,w,h,matrix.Color(i,i,i));
    matrix.show();
  #ifdef AUDIO
    playTone(255-(i/2)-25,1);
  #endif
  }
}

void AwesomeDetected() {
#ifdef DEBUG
  Serial.println("Awesome detected!");
#endif
  t.stop(scan_event);
  detect=false;
  matrix.fillScreen(0);
  matrix.show();
  current_state=AWESOME_DETECTED;
  WipeOut(matrix.Color(0,255,0),100);  
  WipeOut(0,100);
  delay(100);
  scan_event=t.after(5000L,Clear);
}

void Clear() {
#ifdef DEBUG
  Serial.println("Clear...");
#endif
  detect=false;
  WipeOut(matrix.Color(random(255),random(255),random(255)),100);
  WipeOut(matrix.Color(random(255),random(255),random(255)),75);  
  WipeOut(0,50);
  current_state=CLEAR;
  delay(15000);
  startLongRangeScan();
//  scan_event=t.after(15000L,startLongRangeScan);  
}

void getRange() {
    range=pulseIn(SONAR_PIN, HIGH)/147;
    detect=true;
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
  if (lrs_walkers[walker][dest_index]<0 || lrs_walkers[walker][dest_index] >= bound) {
       lrs_walkers[walker][dest_index]=max(0,lrs_walkers[walker][dest_index]);
       lrs_walkers[walker][dest_index]=min(lrs_walkers[walker][dest_index],bound-1);
       lrs_walkers[walker][2]=random(8);
       moveWalker(walker);
  }
}

void moveUp(uint16_t walker) {
  lrs_walkers[walker][1] -=1;
  correctForWall(walker,1,h);
}

void moveDown(uint16_t walker) {
  lrs_walkers[walker][1] +=1;
  correctForWall(walker,1,h); 
}

void moveLeft(uint16_t walker) {
  lrs_walkers[walker][0] -=1;
  correctForWall(walker,0,w);
#ifdef DEBUG
  Serial.print("-- ");Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
#endif
}

void moveRight(uint16_t walker) {
#ifdef DEBUG
  Serial.print("++Right ");Serial.print(walker);Serial.print(" ");
  Serial.print(lrs_walkers[walker][0]);Serial.print(" ");  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
#endif
  lrs_walkers[walker][0] +=1;
  correctForWall(walker,0,w); 
#ifdef DEBUG
  Serial.print("-- ");Serial.print(lrs_walkers[walker][0]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][1]);Serial.print(" "); 
  Serial.print(" ");Serial.print(lrs_walkers[walker][2]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][3]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][4]);Serial.print(" ");
  Serial.print(" ");Serial.print(lrs_walkers[walker][5]);Serial.println(" ");
#endif
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

