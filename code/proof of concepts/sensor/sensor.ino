#include <Arduino.h>

// 8 IR-sensoren op A0..A7
const int sensorPins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};

// Led voor emitter (optioneel)
const int EMITTER_PIN = 6;

void setup() {
  Serial.begin(115200);
  delay(300);

  // sensoren
  for (int i = 0; i < 8; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  // emitter aan
  pinMode(EMITTER_PIN, OUTPUT);
  digitalWrite(EMITTER_PIN, HIGH);

  Serial.println("Sensor test gestart");
}

void loop() {
  Serial.print("S = ");
  for (int i = 0; i < 8; i++) {
    int value = analogRead(sensorPins[i]);
    Serial.print(value);
    Serial.print("  ");
  }
  Serial.println();
  delay(50);
}
