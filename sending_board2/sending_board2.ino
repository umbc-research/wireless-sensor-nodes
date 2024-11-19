#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <Adafruit_MLX90614.h>
//#include <Adafruit_Sensor.h>
//#include <DHT.h>

#define BOARD_ID 1


//MAC Address of the receiver dc:da:0c:21:6e:18

uint8_t broadcastAddress[] = {0xDC, 0xDA, 0x0C, 0x21, 0x6E, 0x18};
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

typedef struct struct_message {
    int id;
    float ambientTemp;
    float objectTemp;
    int readingId;
} struct_message;

esp_now_peer_info_t peerInfo;

struct_message myData;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

unsigned int readingId = 0;

constexpr char WIFI_SSID[] = "UMBC Visitor";

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

float readAmbientTemperature() {
  float ambient = mlx.readAmbientTempC();
  if (isnan(ambient)) {    
    Serial.println("Failed to read from sensor!");
    return 0;
  }
  else {
    Serial.println(ambient);
    return ambient;
  }
}

float readObjectTemperature() {
  float object = mlx.readObjectTempC();
  if (isnan(object)) {    
    Serial.println("Failed to read from sensor!");
    return 0;
  }
  else {
    Serial.println(object);
    return object;
  }
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  Serial.begin(115200);
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while(1);
  };

  WiFi.mode(WIFI_STA);

  int32_t channel = getWiFiChannel(WIFI_SSID);

  WiFi.printDiag(Serial);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial); 

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    myData.id = BOARD_ID;
    myData.ambientTemp = readAmbientTemperature();
    myData.objectTemp = readObjectTemperature();
    myData.readingId = readingId++;
     
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }
}