#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "k";
const char* password = "1234567891011";
bool runAlt = false, runCenter = false;
WebServer server(80);
void handleRoot() {
  server.send(200, "text/html",
    "<h1>ESP32 LED Control</h1>"
    "<button onclick=\"fetch('/press_all')\">Press All</button>"
    "<button onclick=\"fetch('/alt_start')\">Start Alternating</button>"
    "<button onclick=\"fetch('/center_start')\">Start Center</button>"
    "<button onclick=\"fetch('/stop')\">Stop</button>"
  );
}
int getPin(int index) {
  switch (index) {
    case 0: return 2;  
    case 1: return 4;  
    case 2: return 16; 
    case 3: return 17; 
    case 4: return 5;  
  }
  return -1;
} 
void stopPatterns() {
  runAlt = runCenter = false;
  for (int i = 0; i < 5; i++) {
    digitalWrite(getPin(i), LOW);
  }
}
void toggleAll() {
  static bool state = false;
  stopPatterns();
  for (int i = 0; i < 5; i++) {
    digitalWrite(getPin(i), state ? HIGH : LOW);
  }
  state = !state;
  server.send(200,"text/plain","Toggled All LEDs");
}

void startPattern(bool &pattern) {
  stopPatterns();
  pattern = true;
  server.send(200, "text/plain", "Pattern Started");
}

void loopPattern(bool &pattern, int interval, void (*patternFunc)()) {
  static unsigned long lastTime = 0;
  if (pattern && millis() - lastTime > interval) {
    lastTime = millis();
    patternFunc();
  }
}
void alternatingLight() {
  static int step = 0;
  int sequence[] = {0, 2, 4, 1, 3};  
  for (int i = 0; i < 5; i++) {
    digitalWrite(getPin(i), LOW);
  }
  digitalWrite(getPin(sequence[step]), HIGH);
  step = (step + 1) % 5;
}
void centerToBottom() {
  static int step = 0, dir = 1;
  for (int i = 0; i < 5; i++) digitalWrite(getPin(i), LOW);
  for (int i = 0; i <= step; i++) {
    if (2 - i >= 0) digitalWrite(getPin(2 - i), HIGH);
    if (2 + i < 5) digitalWrite(getPin(2 + i), HIGH);
  }
  step += dir;
  if (step > 2 || step < 0) dir *= -1;c
}
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 5; i++) pinMode(getPin(i), OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  server.on("/", handleRoot);
  server.on("/press_all", toggleAll);
  server.on("/alt_start", [](){ startPattern(runAlt); });
  server.on("/center_start", [](){ startPattern(runCenter); });
  server.on("/stop", [](){ stopPatterns(); server.send(200, "text/plain", "Patterns Stopped"); });
  server.begin();
}
void loop() {
  server.handleClient();
  loopPattern(runAlt, 500, alternatingLight);
  loopPattern(runCenter, 300, centerToBottom);
}

