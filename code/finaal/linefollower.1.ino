/* Line follower — PD + WebUI + Flash memory + Start/Stop
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

Preferences prefs;

// ----------------- WiFi -----------------
const char* AP_SSID = "Lineman";
const char* AP_PASS = "12345678";
WebServer server(80);

// ----------------- Pins -----------------
const int sensorPins[8] = {A0,A1,A2,A3,A4,A5,A6,A7};
const int EMITTER_PIN = 6;
const int IN1=2, IN2=3, IN3=4, IN4=5;
const int START_STOP_BUTTON = 7;
const int STATUS_LED = LED_BUILTIN;

// ----------------- PWM -----------------
const int CH_IN1=0, CH_IN2=1, CH_IN3=2, CH_IN4=3;
const int PWM_FREQ=3000, PWM_RES=8;

// ----------------- PD -----------------
float Kp = 2;
float Kd = 140;
int baseSpeed = 90;
int maxSpeed  = 130;

bool isRunning = false;
long lastPosition = 0;
float lastError = 0;


// ================= HTML ===================
String htmlPage() {
  String s =
  "<html><head><meta name='viewport' content='width=device-width,user-scalable=no'>"
  "<style>body{font-family:Arial;padding:15px;}input{width:95%;}</style>"
  "</head><body>"

  "<h2>Line follower</h2>"

  "<button style='width:100%;height:50px;background:#0b0;color:white;font-size:20px;'"
  " onclick='sendControl()'>" + String(isRunning ? "STOP" : "START") + "</button><br><br>"

  "<b>Kp=" + String(Kp) + "</b>"
  "<input type='range' min='0' max='20' step='0.01' value='" + String(Kp) +
  "' onchange='send(\"kp\",this.value)'><br><br>"

  "<b>Kd=" + String(Kd) + "</b>"
  "<input type='range' min='0' max='300' step='1' value='" + String(Kd) +
  "' onchange='send(\"kd\",this.value)'><br><br>"

  "<b>baseSpeed=" + String(baseSpeed) + "</b>"
  "<input type='range' min='0' max='255' step='1' value='" + String(baseSpeed) +
  "' onchange='send(\"base\",this.value)'><br><br>"

  "<b>maxSpeed=" + String(maxSpeed) + "</b>"
  "<input type='range' min='50' max='255' step='1' value='" + String(maxSpeed) +
  "' onchange='send(\"max\",this.value)'><br><br>"

  "<script>"
  "function send(p,v){var x=new XMLHttpRequest();x.open('GET','/set?p='+p+'&v='+v,true);x.send();}"
  "function sendControl(){var x=new XMLHttpRequest();x.open('GET','/toggle',true);x.send();setTimeout(()=>{location.reload()},200);}"
  "</script>"

  "</body></html>";
  return s;
}


// ================= WEB ROUTES ===================
void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSetParam() {
  if (server.hasArg("p") && server.hasArg("v")) {
    String p = server.arg("p");
    float v = server.arg("v").toFloat();

    if (p == "kp")    { Kp = v; prefs.putFloat("kp",Kp); }
    if (p == "kd")    { Kd = v; prefs.putFloat("kd",Kd); }
    if (p == "base")  { baseSpeed = v; prefs.putInt("base",baseSpeed); }
    if (p == "max")   { maxSpeed = v; prefs.putInt("max",maxSpeed); }
  }
  server.send(200, "text/plain", "OK");
}

void handleToggle() {
  isRunning = !isRunning;
  prefs.putBool("run", isRunning);
  server.send(200, "text/plain", "OK");
}


// =============== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(100);

  // Load memory
  prefs.begin("MEM", false);
  Kp = prefs.getFloat("kp", Kp);
  Kd = prefs.getFloat("kd", Kd);
  baseSpeed = prefs.getInt("base", baseSpeed);
  maxSpeed  = prefs.getInt("max", maxSpeed);
  isRunning = prefs.getBool("run", false);

  // WiFi AP
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  // Web
  server.on("/", handleRoot);
  server.on("/set", handleSetParam);
  server.on("/toggle", handleToggle);
  server.begin();

  for (int i=0;i<8;i++) pinMode(sensorPins[i],INPUT);

  pinMode(EMITTER_PIN,OUTPUT); digitalWrite(EMITTER_PIN,HIGH);

  ledcSetup(CH_IN1,PWM_FREQ,PWM_RES); ledcSetup(CH_IN2,PWM_FREQ,PWM_RES);
  ledcSetup(CH_IN3,PWM_FREQ,PWM_RES); ledcSetup(CH_IN4,PWM_FREQ,PWM_RES);
  ledcAttachPin(IN1,CH_IN1); ledcAttachPin(IN2,CH_IN2);
  ledcAttachPin(IN3,CH_IN3); ledcAttachPin(IN4,CH_IN4);

  pinMode(START_STOP_BUTTON,INPUT_PULLUP);
  pinMode(STATUS_LED,OUTPUT);
}


// ============ LINE SENSOR ============
long readLinePosition() {
  long sum=0, wsum=0;
  const int w[8]={-3500,-2500,-1500,-500,500,1500,2500,3500};
  for(int i=0;i<8;i++){int v=analogRead(sensorPins[i]);sum+=v;wsum+= (long)v*w[i];}
  if(sum==0) return lastPosition;
  return lastPosition = wsum/sum;
}


// ============ MOTOR ============
void setMotorPWM(int L,int R){
  L = constrain(L,0,maxSpeed);
  R = constrain(R,0,maxSpeed);
  ledcWrite(CH_IN1,L);  ledcWrite(CH_IN2,0);
  ledcWrite(CH_IN3,R);  ledcWrite(CH_IN4,0);
}


// =============== LOOP =================
void loop() {

  static unsigned long lastWeb=0;
  if (millis()-lastWeb>10){
    server.handleClient();
    lastWeb = millis();
  }

  // HARD start/stop knop
  if (digitalRead(START_STOP_BUTTON)==LOW){
    delay(30);
    if(digitalRead(START_STOP_BUTTON)==LOW){
      isRunning=!isRunning;
      prefs.putBool("run",isRunning);
      while(digitalRead(START_STOP_BUTTON)==LOW) delay(10);
    }
  }

  // STOP
  if(!isRunning){
    digitalWrite(STATUS_LED,LOW);
    setMotorPWM(0,0);
    return;
  }

  digitalWrite(STATUS_LED,HIGH);

  long pos = readLinePosition();
  float err = pos;
  float der = err-lastError;
  lastError=err;

  float out = Kp*err + Kd*der;
  setMotorPWM(baseSpeed + out, baseSpeed - out);
}
