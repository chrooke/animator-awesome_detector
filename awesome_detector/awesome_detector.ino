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
int8_t arraysize = 9;
uint16_t rangevalue[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint16_t sorted_rangevalue[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
State current_state;
boolean detect;
uint16_t range;
Timer t;
int scan_event;

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
  //fill the range array with the first nine values
  for (int i=0;i<arraysize;i++) {
    getRange();
  }
  startLongRangeScan();
}

void loop() {
  switch (current_state) {
    case LONG_RANGE_SCAN:
        if (detect) {
          Serial.print("Range: ");Serial.println(range);
          detect=false;
        }
      break;
    
    case SHORT_RANGE_SCAN:
      break;

    case AWESOME_DETECTED:
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
  current_state=LONG_RANGE_SCAN;
  scan_event=t.every(1000,getRange);
  detect=false;
}

void getRange() {
  int pulse;
  
    pulse=pulseIn(SONAR_PIN, HIGH);
    Serial.print(pulse);Serial.print(" ");Serial.println(pulse/147);
    pulse/=147;
    if (pulse>=10) { //we won't change the view if we read too low or too high
      for (int i=0;i<arraysize-1;++i) { //move all existing array entries down one
        rangevalue[i]=rangevalue[i+1];
      }
      rangevalue[arraysize-1]=pulse;
      for (int i=0;i<arraysize;++i) {
        sorted_rangevalue[i]=rangevalue[i];
      }
      isort(rangevalue,arraysize);        // sort samples
      range = mode(rangevalue,arraysize);  // get median value and convert to inches 
    } 
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
