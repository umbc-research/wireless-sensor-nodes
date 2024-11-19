#include <Adafruit_MLX90614.h>
//#include <Wire.h> //for I2C bus communication

//SCL (serial clock) GOES TO A5
//SDA (serial data) GOES TO A4

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(115200);
  while (!Serial);

  //int sdaPin = 6;
  //int sclPin = 7;
  //Wire.begin(sdaPin, sclPin);

  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1);
  };
}

void loop() {
  Serial.print("Ambient temperature = "); 
  Serial.print(mlx.readAmbientTempC());
  Serial.print("°C");      
  Serial.print("   ");
  Serial.print("Object temperature = "); 
  Serial.print(mlx.readObjectTempC()); 
  Serial.println("°C");
  
  Serial.print("Ambient temperature = ");
  Serial.print(mlx.readAmbientTempF());
  Serial.print("°F");      
  Serial.print("   ");
  Serial.print("Object temperature = "); 
  Serial.print(mlx.readObjectTempF()); 
  Serial.println("°F");

  Serial.println("-----------------------------------------------------------------");
  delay(1000);
}