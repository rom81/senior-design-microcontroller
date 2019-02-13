#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <BH1750FVI.h>
#include <Wire.h>


// Network config
const char* network_name = "network_name";
const char* password = "pass";

// MQTT server config
const char* mqttServer = "mqtt.server.com";
const int mqttPort = 1883;
const char* mqttUser = "user";
const char* mqttPassword = "password";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

void setup()
{
  Serial.begin(115200);
  LightSensor.begin();
 
//  WiFi.begin(network_name, password);
//  Serial.println("about to connect");
//
//  while (WiFi.status() != WL_CONNECTED) 
//  {
//    delay(500);
//    Serial.println("Connecting to WiFi..");
//  }
//  
//  Serial.println("Connected to the WiFi network");
}

void loop()
{
  uint16_t lux = LightSensor.GetLightIntensity();
  Serial.print("Light: ");
  Serial.println(lux);
  delay(250);
}
