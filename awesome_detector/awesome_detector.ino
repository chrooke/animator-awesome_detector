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
  randomSeed(analogRead(0));
  pinMode(SONAR_PIN,INPUT);
  pinMode(SPEAKER_PIN,OUTPUT);
  matrix.begin();
  matrix.setBrightness(10);
  matrix.setTextWrap(false);
  matrix.setTextColor(matrix.Color(255,0,0));
  delay(500); //allow sonar to calibrate and get first reading
  for (uint16_t i; i<SRS_TRANSITION-1;i++) {
    lrs_walkers[i][0]=random(4);
    lrs_walkers[i][1]=random(8);
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
              /*
              Serial.print("Range: ");Serial.println(range);
              Serial.print("upgrade_transition_count: ");Serial.println(upgrade_transition_count);
              */
            }
            detect=false;                  
         }
         if (upgrade_transition_count >= SRS_TRANSITION) {
            startShortRangeScan();
         }
        //move walkers
        matrix.fillScreen(0);
        for (uint16_t i=0;i<=upgrade_transition_count;i++){
/*
          Serial.print(i);Serial.print(" ");
          Serial.print(lrs_walkers[i][0]);Serial.print(" ");
          Serial.print(lrs_walkers[i][1]);Serial.print(" "); 
          Serial.print(lrs_walkers[i][2]);Serial.print(" ");
          Serial.print(lrs_walkers[i][3]);Serial.print(" ");
          Serial.print(lrs_walkers[i][4]);Serial.print(" ");
          Serial.print(lrs_walkers[i][5]);Serial.println(" ");
*/
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
              /*
              Serial.print("Range: ");Serial.println(range);
              Serial.print("downgrade_transition_count: ");Serial.println(downgrade_transition_count);
              Serial.print("upgrade_transition_count: ");Serial.println(upgrade_transition_count); 
              */
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
      delay(100);
      break;

    case CLEAR:
      break;
  }

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
  matrix.show();
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
        delay(1000);
        matrix.drawLine(0,i,w,i,0);
        matrix.drawLine(0,i+1,w,i+1,0);
        matrix.show();
      }
}

