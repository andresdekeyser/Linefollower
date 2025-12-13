/* WiFi WebUI — Parameter control */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

Preferences prefs;
WebServer server(80);

const char* AP_SSID = "Lineman";
const char* AP_PASS = "12345678";

float Kp, Kd;
int baseSpeed, maxSpeed;
bool isRunning;

// ================= HTML =================
String htmlPage() {
  return "<html><body>"
         "<h2>Line follower</h2>"
         "<button onclick='t()'>START/STOP</button><br><br>"
         "Kp<input type='range' min='0' max='20' step='0.01' onchange='s(\"kp\",this.value)'><br>"
         "Kd<input type='range' min='0' max='300' onchange='s(\"kd\",this.value)'><br>"
         "Base<input type='range' min='0' max='255' onchange='s(\"base\",this.value)'><br>"
         "Max<input type='range' min='50' max='255' onchange='s(\"max\",this.value)'><br>"
         "<script>"
         "function s(p,v){fetch(`/set?p=${p}&v=${v}`)}"
         "function t(){fetch('/toggle')}"
         "</script></body></html>";
}

// ================= ROUTES =================
void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSet() {
  String p = server.arg("p");
  float v = server.arg("v").toFloat();

  if(p=="kp")   prefs.putFloat("kp", v);
  if(p=="kd")   prefs.putFloat("kd", v);
  if(p=="base") prefs.putInt("base", v);
  if(p=="max")  prefs.putInt("max", v);

  server.send(200, "text/plain", "OK");
}

void handleToggle() {
  bool r = prefs.getBool("run", false);
  prefs.putBool("run", !r);
  server.send(200, "text/plain", "OK");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  prefs.begin("MEM", false);

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/toggle", handleToggle);
  server.begin();
}

// ================= LOOP =================
void loop() {
  server.handleClient();
}
