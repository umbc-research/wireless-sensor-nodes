#include <DHT.h>
#define DHT22_PIN 2

DHT dht22(DHT22_PIN, DHT22);


void setup() {
  Serial.begin(115200);
  dht22.begin();
}

void loop() {
  delay(2000);

  float humi = dht22.readHumidity();
  float tempC = dht22.readTemperature();

  if (isnan(humi) || isnan(tempC)) {
    Serial.println("Failed to read from DHT22 Sensor!");
  } else {
    Serial.print("DHT22# Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  "); 

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println("°C");
 
  }
}