
// the sevseg display we are using is a common ANODE
// therefore we set the digitPin high, and the segment pin low to enable a segment
// we also use an mpu6050 for bump detection to awaken the robot from its slumber. 

#include "I2Cdev.h"
#include "MPU6050.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

byte digitPins[] = {9, 6, 5}; //Digits on the display: 1,2,3
byte segmentPins[] = {8, 4, 13, 11, 10, 7, A0}; //Segments for each digit: A,B,C,D,E,F,G

/***** INITS *****/

  // MPU6050 //
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

// we use a moving average to smooth out accelerometer readings. 
// we set it to a reasonable starting value to avoid a rise spike when first power on. 
float movingAvgZ = 17000.0;   
float avgAz = 17000.0;

// the trigger threshold for disturbances
float thresh = 1.01;

  // DISPLAY //
int frameNum = 0; 
int robotState = 0; // 0 is sleep
float minsToSleep = 1; // How long should there be no bumps detected before we sleep? 60 seconds

int states = 4;

// how long should the eyes stay open between blinks
// we update this value after each blink
int blinkTime = random(300,5000);
int minStateDuration = 20 * 1000;

unsigned long stateTimer = millis();
unsigned long animTimer = millis();

/***** END INITS *****/

void setup() {

//  Serial.begin(9600);
//  Serial.println("hi");

  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  accelgyro.initialize();
  pinMode(LED_BUILTIN, OUTPUT);
  
  for (int i = 0; i < 3; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
}

void loop() {

  // do mpu6050 stuff
  // get new values and update our moving averages
  accelgyro.getAcceleration(&ax, &ay, &az);
  movingAvgZ += az / 75;
  avgAz += az / 2.5;


  // check if we should go back to sleep
  // check if more than minsToSleep milliseconds has elapsed and we are not already asleep
  if ((millis() - stateTimer > (minsToSleep * 60 * 1000)) && robotState > 1) {
    stateTimer = millis();
    
    // random choice between sleeping and snoring
    robotState = random(0, 1 + 1);
  }

  // if we are bumped and enough time has passed that we are ready to change
  if ((avgAz > movingAvgZ * thresh || avgAz < movingAvgZ / thresh) && (millis() - stateTimer > minStateDuration || robotState == 0)) {
    robotState = random(2, states);
    stateTimer = millis();
  }  

  // manage the state of the circuit based on the robotState value
  switch (robotState) {
    case (0): sleeping();
              break;

    case(1):  if (millis() - animTimer > 1500){
                  frameNum++;
                  animTimer = millis();
              }
              if (frameNum % 4 == 0) {
                 frameNum = 0;
              }
              // the above manages the animation
              snoring(frameNum);
              break;
              
    case (2): bigEyes();
              if (millis() - animTimer > blinkTime) {
                blinkEyes();
                delay(80);
                // delay so that humans can perceive the blink
                animTimer = millis();
                blinkTime = random(300, 5000);
              }
              break;

    case(3): if (frameNum % 2 == 0) {
              if (millis() - animTimer > 2500){ // make the eyes last longer
                frameNum++;
                animTimer = millis();
                }
              } else {
                if (millis() - animTimer > 300){ // shorten the transition
                  frameNum++;
                  animTimer = millis();
                }
              }
              if (frameNum % 4 == 0) {
                 frameNum = 0;
              }
              suspicious_look(frameNum);
              break;
              
    case (4): if (millis() - animTimer > 100){
                  frameNum++;
                  animTimer = millis();
                  if (frameNum % 4 == 0) {
                     frameNum = 0;
                  }
              }
              mouth_woo(frameNum);
              break;
  }

//  Serial.print(movingAvgZ); Serial.print("\t");
//  Serial.print(movingAvgZ * thresh); Serial.print("\t");
//  Serial.print(avgAz); Serial.print("\t");
//  Serial.println(movingAvgZ / thresh);

  movingAvgZ -= movingAvgZ / 75.0;
  avgAz -= avgAz / 2.5;

}

//***** below are the functions used to manage the lighting *****//

// function to set which numerals to use.
// pass it an array corresponding to [d1, d2, d3, d4], 0 off, 1 on.
void setDisplayOn(byte disp[3]) {
  for (int i = 0; i < 3; i++) {
    if (disp[i] == 1) {
      digitalWrite(digitPins[i], HIGH);
    }
  }
}

void displayAllOff() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(digitPins[i], LOW);
  }
}

void setSeg(char seg) {
  switch (tolower(seg)) {
    case ('a') : digitalWrite(segmentPins[0], LOW);
      break;
    case ('b') : digitalWrite(segmentPins[1], LOW);
      break;
    case ('c') : digitalWrite(segmentPins[2], LOW);
      break;
    case ('d') : digitalWrite(segmentPins[3], LOW);
      break;
    case ('e') : digitalWrite(segmentPins[4], LOW);
      break;
    case ('f') : digitalWrite(segmentPins[5], LOW);
      break;
    case ('g') : digitalWrite(segmentPins[6], LOW);
      break;
  }
  delay(1);
}

void setSegGroup(char group[], size_t group_size) {
  for (int i = 0; i < group_size; i++) {
    setSeg(group[i]);
  }
}

void clearDisplay() {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], HIGH);
  }
}

void bigEyes() {
  byte segDisp[] = {1, 0, 1};
  char bigEye[] = {'a', 'b', 'c', 'd', 'e', 'f'};

  displayAllOff();
  setDisplayOn(segDisp);
  clearDisplay();

  setSegGroup(bigEye, 6);
}

void smallEyesLow() {
  byte segDisp[] = {1, 0, 1};
  char smallEye[] = {'c', 'd', 'e', 'g'};

  displayAllOff();
  setDisplayOn(segDisp);
  clearDisplay();

  setSegGroup(smallEye, 4);
}

void smallEyesHigh() {
  byte segDisp[] = {1, 0, 1};
  char smallEye[] = {'a', 'b', 'f', 'g'};

  displayAllOff();
  setDisplayOn(segDisp);
  clearDisplay();

  setSegGroup(smallEye, 4);
}

void eyebrowEyes() {
  byte segDisp[] = {1, 0, 1};
  char smallEye[] = {'c', 'd', 'e', 'g'};

  displayAllOff();
  setDisplayOn(segDisp);
  clearDisplay();

  setSeg('a');
  setSegGroup(smallEye, 4);
}

void smallEyeSmile() {
  byte segDispEyes[] = {1, 0, 1};
  byte segDispSmile[] = {0, 1, 0};
  char smallEyeHigh[] = {'a', 'b', 'f', 'g'};

  clearDisplay();
  displayAllOff();

  setDisplayOn(segDispEyes);
  setSegGroup(smallEyeHigh, 4);

  delay(1);

  displayAllOff();
  clearDisplay();
  setDisplayOn(segDispSmile);
  setSeg('d');

}

void bigEyeSmile() {
  byte segDispEyes[] = {1, 0, 1};
  byte segDispSmile[] = {0, 1, 0};
  char bigEye[] = {'a', 'b', 'c', 'd', 'e', 'f'};

  clearDisplay();
  displayAllOff();

  setDisplayOn(segDispEyes);
  setSegGroup(bigEye, 6);

  delay(1);

  displayAllOff();
  clearDisplay();
  setDisplayOn(segDispSmile);
  setSeg('d');

}

void blinkEyes() {
  byte segDispEyes[] = {1, 0, 1};

  displayAllOff();
  clearDisplay();
  setDisplayOn(segDispEyes);
  setSeg('g');

}

void sleeping() {
  byte segDispEyes[] = {1, 0, 1};
  byte segDispSmile[] = {0, 1, 0};

  clearDisplay();
  displayAllOff();

  setDisplayOn(segDispEyes);
  setSeg('g');

  delay(1);

  displayAllOff();
  clearDisplay();
  setDisplayOn(segDispSmile);
  setSeg('d');
}

void mouth_woo(int frame) {
  byte segDispEyes[] = {1, 0, 1};
  byte segDispSmile[] = {0, 1, 0};
  char bigEye[] = {'a', 'b', 'c', 'd', 'e', 'f'};


  clearDisplay();
  displayAllOff();

  setDisplayOn(segDispEyes);
  setSegGroup(bigEye, 6);
  
  displayAllOff();
  clearDisplay();
  setDisplayOn(segDispSmile);

  switch (frame) {
    case (0):   
                setSeg('d');
                delay(5);
                break;
                
    case (1):   
                setSeg('e');
                delay(10);
                break;
                
    case (2):   setSeg('g');
                delay(10);
                break;

    case(3):    setSeg('c');
                delay(10);
                
  }
}

void suspicious_look(int frame) {
  byte segDispEyesApart[] = {1, 0, 1};
  byte segDispEyesLeft[] = {1, 1, 0};
  byte segDispEyesRight[] = {0, 1, 1};
  char smallEyeWithBrow[] = {'a', 'c', 'd', 'e', 'g'};

  displayAllOff();
  clearDisplay();

  switch (frame) {  
    case (0): setDisplayOn(segDispEyesLeft);
              break;
              
    case (1): setDisplayOn(segDispEyesApart);
              break;
              
    case (2): setDisplayOn(segDispEyesRight);
              break;
              
    case (3): setDisplayOn(segDispEyesApart);
              break;
  }

  setSegGroup(smallEyeWithBrow, 5);
}

void snoring(int frame) {
  byte segDispEyes[] = {1, 0, 1};
  byte segDispSmile[] = {0, 1, 0};

  byte mouthSmall[] = {'c', 'd', 'e', 'g'};
  byte mouthBig[] = {'a', 'b', 'c', 'd', 'e', 'f'};

  clearDisplay();
  displayAllOff();
  
  setDisplayOn(segDispEyes);
  setSeg('g');

  delay(1);

  displayAllOff();
  clearDisplay();
  setDisplayOn(segDispSmile);

  switch (frame) {  
  case (0): setSeg('d');
            break;
            
  case (1): setSegGroup(mouthSmall, 4);
            break;

  case (2): setSegGroup(mouthBig, 6);
            delay(30);
            break;
            
  case (3): setSegGroup(mouthSmall, 4);
            break;
  }
}
