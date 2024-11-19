#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

const char* ssid = "UMBC Visitor";
const char* password = "";


typedef struct struct_message {
  int id;
  float ambientTemp;
  float objectTemp;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;
JSONVar board;

AsyncWebServer server(80);
AsyncEventSource events("/events");

// Data reception callback
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Packet received from: ");
  Serial.println(macStr);

  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  board["id"] = incomingReadings.id;
  board["ambientTemperature"] = incomingReadings.ambientTemp;  // Match JavaScript field names
  board["objectTemperature"] = incomingReadings.objectTemp;    // Match JavaScript field names
  board["readingId"] = String(incomingReadings.readingId);

  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());

  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("Ambient_temp value: %4.2f \n", incomingReadings.ambientTemp);
  Serial.printf("Object_temp value: %4.2f \n", incomingReadings.objectTemp);
  Serial.printf("readingID value: %d \n", incomingReadings.readingId);
  Serial.println();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>UMBC Sensor Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css">
  <style>
    html { font-family: Arial; text-align: center; }
    p { font-size: 1.2rem; }
    body { margin: 0; }
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px rgba(140,140,140,0.5); padding: 15px; }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
    .ambient { color: #fd7e14; }
    .object { color: #1b78e2; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>UMBC Sensor Dashboard</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card ambient">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #1 - AMBIENT TEMPERATURE</h4>
        <p><span class="reading"><span id="ambient1"></span> &deg;C</span></p>
        <p class="packet">Reading ID: <span id="ra1"></span></p>
      </div>
      <div class="card object">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #1 - OBJECT TEMPERATURE</h4>
        <p><span class="reading"><span id="object1"></span> &deg;C</span></p>
        <p class="packet">Reading ID: <span id="ro1"></span></p>
      </div>
      <div class="card ambient">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #2 - AMBIENT TEMPERATURE</h4>
        <p><span class="reading"><span id="ambient2"></span> &deg;C</span></p>
        <p class="packet">Reading ID: <span id="ra2"></span></p>
      </div>
      <div class="card object">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #2 - OBJECT TEMPERATURE</h4>
        <p><span class="reading"><span id="object2"></span> &deg;C</span></p>
        <p class="packet">Reading ID: <span id="ro2"></span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("ambient" + obj.id).innerHTML = obj.ambientTemperature.toFixed(2);
    document.getElementById("object" + obj.id).innerHTML = obj.objectTemperature.toFixed(2);
    document.getElementById("ra" + obj.id).innerHTML = obj.readingId;
    document.getElementById("ro" + obj.id).innerHTML = obj.readingId;
  }, false);
}
</script>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  
  events.onConnect([](AsyncEventSourceClient *client){
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {
  static unsigned long lastEventTime = millis();
  const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping", NULL, millis());
    lastEventTime = millis();
  }
}
