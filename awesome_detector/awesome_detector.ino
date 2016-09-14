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
#define SRS_TRANSITION 5
#define AD_TRANSITION 15


enum State {
  LONG_RANGE_SCAN,
  SHORT_RANGE_SCAN,
  AWESOME_DETECTED,
  CLEAR
};

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(4, 8, MATRIX_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);


int w    = matrix.width();
int h    = matrix.height();
int rx,ry,rr,rg,rb;
State current_state;
boolean detect;
uint16_t range;
Timer t;
int scan_event;
int srs_transition_count;
int lrs_transition_count;
int ad_transition_count;

//holds the "walkers" for the long range scanner.
uint16_t lrs_walkers[SRS_TRANSITION-1][5]; 
 
void setup() {
  Serial.begin(9600);
  Serial.println("Setting up...");
  pinMode(SONAR_PIN,INPUT);
  pinMode(SPEAKER_PIN,OUTPUT);
  matrix.begin();
  matrix.setBrightness(10);
  matrix.fillScreen(0);
  matrix.show();
  delay(500); //allow sonar to calibrate and get first reading
  startLongRangeScan();
}

void loop() {
  switch (current_state) {
    case LONG_RANGE_SCAN:
        if (detect) {
            if (range>90) {
              srs_transition_count=0;
            } else if (range>60) {
              srs_transition_count--;
              srs_transition_count=max(0,srs_transition_count);
            } else if (range>36) {
              srs_transition_count++;
            } else if (range>12) {
              srs_transition_count+=2;
            }
            if (range>12) {
              Serial.print("Range: ");Serial.println(range);
              Serial.print("srs_transition_count: ");Serial.println(srs_transition_count);
            }
            detect=false;                  
         }
         if (srs_transition_count >= SRS_TRANSITION) {
          startShortRangeScan();
         }
        break;
    
    case SHORT_RANGE_SCAN:
        if (detect) {
            if (range>40) {
              lrs_transition_count++;
              ad_transition_count=0;
            } else if (range>30) {
              ad_transition_count--;
              ad_transition_count=max(0,ad_transition_count);
            } else if (range>20) {
              lrs_transition_count--;
              lrs_transition_count=max(0,lrs_transition_count);
              ad_transition_count++;
            } else if (range>12) {
              lrs_transition_count--;
              lrs_transition_count=max(0,lrs_transition_count);
              ad_transition_count++;
            }
            if (range>12) {
              Serial.print("Range: ");Serial.println(range);
              Serial.print("lrs_transition_count: ");Serial.println(lrs_transition_count);
              Serial.print("ad_transition_count: ");Serial.println(ad_transition_count); 
            }
            detect=false;
         }
         if (lrs_transition_count >= LRS_TRANSITION) {
            startLongRangeScan();
         }
         if (ad_transition_count >= AD_TRANSITION) {
            AwesomeDetected();
         }
      break;

    case AWESOME_DETECTED:
      matrix.fillScreen(matrix.Color(255, 0, 0));
      matrix.show();
      break;

    case CLEAR:
      matrix.fillScreen(0);
      matrix.show();
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
  current_state=LONG_RANGE_SCAN;
  srs_transition_count=0;  
  scan_event=t.every(1000,getRange);

}

void startShortRangeScan() {
  Serial.println("Switching to short range scanner...");
  t.stop(scan_event);
  detect=false;
  current_state=SHORT_RANGE_SCAN;
  lrs_transition_count=0; 
  ad_transition_count=0;
  scan_event=t.every(250,getRange);
}

void AwesomeDetected() {
  Serial.println("Awesome detected!");
  t.stop(scan_event);
  detect=false;
  current_state=AWESOME_DETECTED;
  scan_event=t.after(5000L,Clear);
}

void Clear() {
  Serial.println("Clear...");
  detect=false;
  current_state=CLEAR;
  scan_event=t.after(15000L,startLongRangeScan);  
}

void getRange() {
    range=pulseIn(SONAR_PIN, HIGH)/147;
    detect=true;
}



// Sorting function (Author: Bill Gentles, Nov. 12, 2010)
void isort(uint16_t *a, int8_t n){
  for (int i = 1; i < n; ++i)  {
    uint16_t j = a[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < a[k]); k--) {
      a[k + 1] = a[k];
    }
    a[k + 1] = j;
  }
}
 
// Mode function, returning the mode or median.
uint16_t mode(uint16_t *x,int n){
  int i = 0;
  int count = 0;
  int maxCount = 0;
  uint16_t mode = 0;
  int bimodal;
  int prevCount = 0;
  while(i<(n-1)){
    prevCount=count;
    count=0;
    while( x[i]==x[i+1] ) {
      count++;
      i++;
    }
    if( count > prevCount & count > maxCount) {
      mode=x[i];
      maxCount=count;
      bimodal=0;
    }
    if( count == 0 ) {
      i++;
    }
    if( count == maxCount ) {      //If the dataset has 2 or more modes.
      bimodal=1;
    }
    if( mode==0 || bimodal==1 ) {  // Return the median if there is no mode.
      mode=x[(n/2)];
    }
    return mode;
  }
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
