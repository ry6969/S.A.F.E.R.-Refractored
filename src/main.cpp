#include <Arduino.h>
#include <Servo.h>
#include <TimedAction.h>

#define FLAME_PIN A1
#define GAS_PIN A0
#define SERVO1_PIN 9
#define SERVO2_PIN 6
#define BUZZER_PIN 8
#define BLUE_LED_PIN 5
#define RED_LED_PIN 4
#define ORANGE_LED_PIN 3
#define PUMP_RELAY_PIN 2

//Component Object Constructor
Servo servo1;
Servo servo2;

//Function Declaration
int readFlameLevel();
bool isFlamePresent(int flameLevel);
bool calculateFlicker();
void printFlameStatus(bool flameDetected, int flameLevel);
int readSmokeLevel();
bool isSmokePresent(int smokeLevel);
void printSmokeStatus(int smokeLevel, bool smokeDetected);
void sweepServo(Servo &servo, int &currentAngle, bool &reachedMax, int minAngle, int maxAngle, int angleDifference);

// LED & Buzzer declarations
void bootDisplay();
void blinkALL(int count, int interval);
void LedOn(char color);
void LedOff(char color);
void buzzerAlert(char type);

//Global Variable
int minAngle = 40;
int maxAngle = 150;

// Timer Variables;
unsigned long servo1_Timer = 0;
unsigned long servo2_Timer = 0;
unsigned long debug_Timer = 0;

// Servo state variables
int servo1_currentAngle = 90;
bool servo1_reachedMax = false;

int servo2_currentAngle = servo1_currentAngle;
bool servo2_reachedMax = false;

void setup(){

  Serial.begin(9600);
  Serial.println("Device Starting");

  //Pin Set-ups
  pinMode(FLAME_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(SERVO1_PIN, OUTPUT);
  pinMode(SERVO2_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  //Make sure to turnoff the water pump 
  digitalWrite(PUMP_RELAY_PIN, HIGH);

  bootDisplay();
  blinkALL(2, 250);
  //LedOn('B');
  delay(500);

  Serial.println("Set-up is complete and ready!!!");


}

void loop(){

  //Time update
  unsigned long now = millis();

  //Sensor Reading
  int flameLevel= readFlameLevel();
  bool flameDetected = isFlamePresent(flameLevel);
  delay(50);  // Let ADC settle between different analog pins
  int smokeLevel = readSmokeLevel();
  bool smokeDetected = isSmokePresent(smokeLevel);

  if(timedAction(debug_Timer, 250, now)){
    printFlameStatus(flameDetected, flameLevel);
    printSmokeStatus(smokeLevel, smokeDetected);
  }

  //detect flame & smoke (TODO: Add smoke detection later)
  if (!flameDetected && !smokeDetected){
    if (timedAction(servo1_Timer, 30, now)){
      sweepServo(servo1, servo1_currentAngle, servo1_reachedMax, minAngle, maxAngle, 5);
    }

    // Return to the middle before turning off the pump
    servo2.write(100);
    digitalWrite(PUMP_RELAY_PIN, HIGH);
    LedOff('R');
    LedOff('O');
    digitalWrite(BUZZER_PIN, LOW);
  } 

  if (flameDetected && smokeDetected){
    LedOn('R');
    LedOn('O');
    buzzerAlert('S');
    digitalWrite(PUMP_RELAY_PIN, LOW);
    if (timedAction(servo2_Timer, 50, now)){
      sweepServo(servo2, servo2_currentAngle, servo2_reachedMax, minAngle, maxAngle, 5);
    }
  }

  if (flameDetected){
      int targetAngle = servo1_currentAngle + 10;
      Serial.println(targetAngle);
      digitalWrite(PUMP_RELAY_PIN, LOW);
      printFlameStatus(flameDetected, flameLevel);
      while(!timedAction(servo1_Timer, 5000, now)){
        LedOn('R');
        buzzerAlert('F');
        if (timedAction(servo2_Timer, 30, now)){
          sweepServo(servo2, servo2_currentAngle, servo2_reachedMax, targetAngle - 20, targetAngle + 20, 5); 
        }
        now = millis();
      }
  }

  if (smokeDetected){
    LedOn('O');
    buzzerAlert('S');
    digitalWrite(PUMP_RELAY_PIN, LOW);
    if (timedAction(servo2_Timer, 50, now)){
      sweepServo(servo2, servo2_currentAngle, servo2_reachedMax, minAngle, maxAngle, 5);
    }
  }
  
}

//===================
//  Flame Detection
//===================
int readFlameLevel(){
  int flameLevel = analogRead(FLAME_PIN);
  return flameLevel;
}

bool isFlamePresent(int flameLevel){
  if (flameLevel < 1000){
    if(calculateFlicker()){
      return true;
    } else return false;
  } else return false;
}

bool calculateFlicker() {
  const int samples = 15;                   // number of samples
  const float threshold = 10;          // adjust this based on testing
  int readings[samples];

  // 1. Collect samples
  for (int i = 0; i < samples; i++) {
    readings[i] = analogRead(FLAME_PIN);
    delay(3);                      // helps capture flicker; edit as needed
  }

  //Compute mean
  float sum = 0;
  for (int i = 0; i < samples; i++) sum += readings[i];
  float mean = sum / samples;

  //Compute variance
  float varSum = 0;
  for (int i = 0; i < samples; i++) {
    float diff = readings[i] - mean;
    varSum += diff * diff;
  }
  float variance = varSum / samples;

  //Standard Deviation
  float sd = sqrt(variance);
    

  //Decision
  return (sd > threshold);
}

void printFlameStatus(bool flameDetected, int flameLevel){
  Serial.print("Flame Value: ");
  Serial.print(flameLevel);
  Serial.println(flameDetected ? " Status: Flame Detected!!!" : " Status: No Flame Detected");
}

//======================
//   Smoke Detection
//======================
int readSmokeLevel(){
  int smokeLevel = analogRead(GAS_PIN);
  return smokeLevel;
}

bool isSmokePresent(int smokeLevel){
  if (smokeLevel > 280){
    return true;
  } else return false;
}

void printSmokeStatus(int smokeLevel, bool smokeDetected){
  Serial.print("Smoke Value: ");
  Serial.print(smokeLevel);

  if (smokeDetected){
    Serial.println(" Status: Smoke Dectected!!!");
  } else {
    Serial.println(" Status: No Smoke Detected");
  }
}

//=====================
//    SERVO CONTROL
//=====================

void sweepServo(Servo &servo, int &currentAngle, bool &reachedMax, int minAngle, int maxAngle, int angleDifference){
  if (!reachedMax) {
    currentAngle += angleDifference;
    if (currentAngle >= maxAngle) reachedMax = true;
  } else {
    currentAngle -= angleDifference;
    if (currentAngle <= minAngle) reachedMax = false;
  }
  servo.write(currentAngle);
}

//===================
//    LED Control
//===================
void bootDisplay(){
  // Runs once at startup; blocking is acceptable here
  //LedOn('O'); delay(250);
  //LedOn('R'); delay(250);
  //LedOn('B'); delay(250);
  LedOff('O'); LedOff('R'); LedOff('B');
  delay(250);
}

void LedOn(char color){
  if (color == 'B')      digitalWrite(BLUE_LED_PIN, HIGH);
  else if (color == 'R') digitalWrite(RED_LED_PIN, HIGH);
  else if (color == 'O') digitalWrite(ORANGE_LED_PIN, HIGH);
}

void LedOff(char color){
  if (color == 'B')      digitalWrite(BLUE_LED_PIN, LOW);
  else if (color == 'R') digitalWrite(RED_LED_PIN, LOW);
  else if (color == 'O') digitalWrite(ORANGE_LED_PIN, LOW);
}

void blinkALL(int count, int interval){
  for(int i = 0; i < count; ++i){
    //LedOn('B'); LedOn('R'); LedOn('O');
    delay(interval);
    LedOff('B'); LedOff('R'); LedOff('O');
    delay(interval);
  }
}

//===========================
//      Buzzer Control
//===========================
// type: 'S' smoke (slow), 'F' fire (fast)
void buzzerAlert(char type){
  static unsigned long smokeTimer = 0; // self-contained timers for timedAction
  static unsigned long fireTimer  = 0;
  static bool smokeState = false;
  static bool fireState  = false;
  unsigned long now = millis();

  if(type == 'S'){
    if(timedAction(smokeTimer, 250, now)){
      smokeState = !smokeState;
      digitalWrite(BUZZER_PIN, smokeState ? HIGH : LOW);
    }
  } else if(type == 'F'){
    if(timedAction(fireTimer, 100, now)){
      fireState = !fireState;
      digitalWrite(BUZZER_PIN, fireState ? HIGH : LOW);
    }
  } else {
    // Unknown type -> ensure buzzer off
    digitalWrite(BUZZER_PIN, LOW);
  }
}