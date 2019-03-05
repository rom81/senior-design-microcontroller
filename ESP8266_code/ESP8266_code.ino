#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <BH1750FVI.h>
#include <Wire.h>


// Network config
//const char* ssid = "WIRELESS-PITTNET";
//const char* password = "pass";
const char* ssid = "WIRELESS-PITTNET";
const char* security_type = "WPA-Enterprise";
const char* encryption_type = "TKIP";
const char* auth_method = "PEAP";
const char* username = "rom81";
const char* password = "";

// MQTT server config
const char* mqttServer = "54.158.107.32";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

void setup()
{
  Serial.begin(115200);
  LightSensor.begin();
 
  WiFi.begin(ssid, password);
  Serial.println("about to connect");

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println("Connected to the WiFi network");
}

void loop()
{
  uint16_t lux = LightSensor.GetLightIntensity();
  Serial.print(lux);
  Serial.println(" lux");
  delay(250);
}
