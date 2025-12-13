/* Line follower — PD controller
   Nano ESP32 + DRV8833 + 8x analog IR sensor (A0..A7)
   Emitter on D6
   Motors: IN1=D2, IN2=D3, IN3=D4, IN4=D5
   Start/Stop button: D7
   Calibrate button: D8
*/

#include <Arduino.h>

// --- pins ---
const int sensorPins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
const int EMITTER_PIN = 6;    // D6

// DRV8833 IN pins
const int IN1 = 2; 
const int IN2 = 3; 
const int IN3 = 4; 
const int IN4 = 5; 

const int START_STOP_BUTTON = 7; 
const int CALIBRATE_BUTTON  = 8; 
const int STATUS_LED = LED_BUILTIN;

// --- PWM ---
const int CH_IN1 = 0;
const int CH_IN2 = 1;
const int CH_IN3 = 2;
const int CH_IN4 = 3;
const int PWM_FREQ = 3000;
const int PWM_RES  = 8;

// --- PD parameters ---
float Kp = 2;
float Kd = 140;     // ← derivative gain (tune this)
int baseSpeed = 90;
int maxSpeed  = 130;

// runtime
long lastPosition = 0;
float lastError = 0;
bool isRunning = false;

void setup() {
  Serial.begin(115200);
  delay(200);

  for (int i = 0; i < 8; ++i) pinMode(sensorPins[i], INPUT);

  pinMode(EMITTER_PIN, OUTPUT);
  digitalWrite(EMITTER_PIN, HIGH);

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

  pinMode(START_STOP_BUTTON, INPUT_PULLUP);
  pinMode(CALIBRATE_BUTTON, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);

  digitalWrite(STATUS_LED, LOW);

  Serial.println("PD line follower ready.");
}

// --- weighted average line position ---
long readLinePosition() {
  long weightedSum = 0;
  long sumVals = 0;

  const int weights[8] = {-3500, -2500, -1500, -500, 500, 1500, 2500, 3500};

  for (int i = 0; i < 8; ++i) {
    int v = analogRead(sensorPins[i]);
    sumVals += v;
    weightedSum += (long)v * weights[i];
  }

  if (sumVals == 0) return lastPosition;

  long pos = weightedSum / sumVals;
  lastPosition = pos;
  return pos;
}

// --- motor control ---
void setMotorPWM(int leftPWM, int rightPWM) {
  if (leftPWM < 0) leftPWM = 0;
  if (rightPWM < 0) rightPWM = 0;
  if (leftPWM > maxSpeed) leftPWM = maxSpeed;
  if (rightPWM > maxSpeed) rightPWM = maxSpeed;

  ledcWrite(CH_IN1, leftPWM);
  ledcWrite(CH_IN2, 0);
  ledcWrite(CH_IN3, rightPWM);
  ledcWrite(CH_IN4, 0);
}

void loop() {

  // --- start/stop ---
  if (digitalRead(START_STOP_BUTTON) == LOW) {
    delay(40);
    if (digitalRead(START_STOP_BUTTON) == LOW) {
      isRunning = !isRunning;
      Serial.println(isRunning ? "RUN" : "STOP");
      while (digitalRead(START_STOP_BUTTON) == LOW) delay(10);
    }
  }

  if (!isRunning) {
    setMotorPWM(0, 0);
    digitalWrite(STATUS_LED, LOW);
    delay(20);
    return;
  }

  digitalWrite(STATUS_LED, HIGH);

  // --- read sensors ---
  long position = readLinePosition(); // -3500..+3500
  float error = (float)position;      

  // --- PD CONTROL ---
  float derivative = error - lastError;
  lastError = error;

  float output = Kp * error + Kd * derivative;

  // motor mixing
  float leftF  = (float)baseSpeed + output;
  float rightF = (float)baseSpeed - output;

  int leftPWM  = constrain(leftF, 0, maxSpeed);
  int rightPWM = constrain(rightF, 0, maxSpeed);

  setMotorPWM(leftPWM, rightPWM);

  // debug
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 300) {
    Serial.print("pos=");  Serial.print(position);
    Serial.print(" e=");    Serial.print(error);
    Serial.print(" d=");    Serial.print(derivative);
    Serial.print(" out=");  Serial.print(output);
    Serial.print(" L=");    Serial.print(leftPWM);
    Serial.print(" R=");    Serial.println(rightPWM);
    lastPrint = millis();
  }
  delay(1);

}
