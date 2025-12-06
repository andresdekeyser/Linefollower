

#include <Arduino.h>


const int IN1 = 2; 
const int IN2 = 3; 
const int IN3 = 4; 
const int IN4 = 5;

const int CH_IN1 = 0;
const int CH_IN2 = 1;
const int CH_IN3 = 2;
const int CH_IN4 = 3;
const int PWM_FREQ = 3000;
const int PWM_RES  = 8;


// =====================================
void setupHbridge() {

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(CH_IN1, PWM_FREQ, PWM_RES);
  ledcSetup(CH_IN2, PWM_FREQ, PWM_RES);
  ledcSetup(CH_IN3, PWM_FREQ, PWM_RES);
  ledcSetup(CH_IN4, PWM_FREQ, PWM_RES);

  ledcAttachPin(IN1, CH_IN1);
  ledcAttachPin(IN2, CH_IN2);
  ledcAttachPin(IN3, CH_IN3);
  ledcAttachPin(IN4, CH_IN4);

  ledcWrite(CH_IN1, 0);
  ledcWrite(CH_IN2, 0);
  ledcWrite(CH_IN3, 0);
  ledcWrite(CH_IN4, 0);
}


// =====================================
void setMotorPWM(int leftPWM, int rightPWM) {

  if (leftPWM < 0) leftPWM = 0;
  if (rightPWM < 0) rightPWM = 0;
  if (leftPWM > 255) leftPWM = 255;
  if (rightPWM > 255) rightPWM = 255;

  ledcWrite(CH_IN1, leftPWM);
  ledcWrite(CH_IN2, 0);
  ledcWrite(CH_IN3, rightPWM);
  ledcWrite(CH_IN4, 0);
}
