/* Start/Stop Interrupt Controller */

#include <Arduino.h>
#include <Preferences.h>

Preferences prefs;

const int START_STOP_BUTTON = 7;
const int STATUS_LED = LED_BUILTIN;

volatile bool toggleRequest = false;
unsigned long lastISR = 0;

// ===== ISR =====
void IRAM_ATTR onButtonPress() {
  unsigned long now = millis();
  if(now - lastISR > 200){   // debounce
    toggleRequest = true;
    lastISR = now;
  }
}

void setup() {
  Serial.begin(115200);
  prefs.begin("MEM", false);

  pinMode(START_STOP_BUTTON, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);

  attachInterrupt(
    digitalPinToInterrupt(START_STOP_BUTTON),
    onButtonPress,
    FALLING
  );
}

void loop() {

  if(toggleRequest){
    toggleRequest = false;

    bool run = prefs.getBool("run", false);
    run = !run;
    prefs.putBool("run", run);

    digitalWrite(STATUS_LED, run);
    Serial.println(run ? "RUN" : "STOP");
  }

  delay(10);
}
