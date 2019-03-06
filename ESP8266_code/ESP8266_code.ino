#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <BH1750FVI.h>
#include <Wire.h>
#include <inttypes.h>

#define LUMOSITY_DELTA_THRESHOLD 50
#define UUID "1fb54a9a-75ef-4962-b703-a6304be18e65"

// MQTT topics
#define JOIN             "ambiance/system/join"

#define SETTINGS_SET     "ambiance/device/1fb54a9a-75ef-4962-b703-a6304be18e65/settings/set"
#define SETTINGS_REQUEST "ambiance/device/1fb54a9a-75ef-4962-b703-a6304be18e65/settings/request"
#define SETTINGS         "ambiance/device/1fb54a9a-75ef-4962-b703-a6304be18e65/settings"

#define DATA_REQUEST     "ambiance/device/1fb54a9a-75ef-4962-b703-a6304be18e65/data/request"
#define DATA             "ambiance/device/1fb54a9a-75ef-4962-b703-a6304be18e65/data" 

// Network config
const char* network_name = "Embedded Systems Class";
const char* password = "embedded1234";

// MQTT server config
const char* mqttServer = "ec2-54-158-107-32.compute-1.amazonaws.com";
const int mqttPort = 1883;
const char* mqttUser = "";  // currently has no username
const char* mqttPassword = ""; // currently has no password

WiFiClient wifiClient;
PubSubClient client(wifiClient);

BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

int curr_lux, prev_lux;

void setup()
{
  curr_lux = 0;
  prev_lux = 0;
  
  Serial.begin(115200);

  // Initialize BH1750 sensor
  LightSensor.begin();

  // TODO: Initialize TCS sensor
  

  // Connect to WiFi network
  wifi_connect();
  
  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  mqtt_connect();

  // Subscribe and publish to JOIN 
  if (client.subscribe(JOIN)) 
    Serial.println("Subscribed to JOIN");
  if (client.publish(JOIN, UUID)) 
    Serial.println("Published device UUID to JOIN");
  
  // Subscribe and publish to DATA 
  if (client.subscribe(DATA)) 
    Serial.println("Subscribed to DATA");
//  if (client.publish(DATA, "Sensor connected")) 
//    Serial.println("Published init message to DATA");

  // Subscribe and publish to SETTINGS_SET
  if (client.subscribe(SETTINGS_SET)) 
    Serial.println("Subscribed to SETTINGS_SET");
//  if (client.publish(SETTINGS_SET, "Sensor connected")) 
//    Serial.println("Published init message to SETTINGS_SET");

  // Subscribe and publish to SETTINGS_REQUEST
  if (client.subscribe(SETTINGS_REQUEST)) 
    Serial.println("Subscribed to SETTINGS_REQUEST");
//  if (client.publish(SETTINGS_REQUEST, "Sensor connected")) 
//    Serial.println("Published init message to SETTINGS_REQUEST");

  // Subscribe and publish to SETTINGS
  if (client.subscribe(SETTINGS)) 
    Serial.println("Subscribed to SETTINGS");
//  if (client.publish(SETTINGS, "Sensor connected")) 
//    Serial.println("Published init message to SETTINGS");

  // Subscribe and publish to DATA_REQUEST
  if (client.subscribe(DATA_REQUEST)) 
    Serial.println("Subscribed to DATA_REQUEST");
//  if (client.publish(DATA_REQUEST, "Sensor connected")) 
//    Serial.println("Published init message to DATA_REQUEST");

}

void loop()
{
  curr_lux = LightSensor.GetLightIntensity();

  // Significant change in light intensity, send update to server
  if (abs(curr_lux - prev_lux) >= LUMOSITY_DELTA_THRESHOLD)
  {
    char payload[256];
    sprintf(payload, "%d", curr_lux);
    
    // publish new message
    if (client.publish(DATA, payload)) 
      Serial.println("Published new lux reading to DATA"); 
  }
  
  if (!client.loop())
  {
    // Try to reconnect to MQTT server
    Serial.println("Lost connection to the mqtt server");
    mqtt_connect();
  }
  
  prev_lux = curr_lux;
}

//void publish_lux()
//{
//  int lux = LightSensor.GetLightIntensity();
//  char payload[256];
//  sprintf(payload, "%d", lux);
//  if (client.publish(DATA, payload))
//    Serial.println("Published new lux reading to DATA");
//}

void wifi_connect()
{
  WiFi.begin(network_name, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void mqtt_connect()
{
  while (!client.connected())
  {
    Serial.println("Connecting to MQTT server...");

    if (client.connect("MQTTServer", mqttUser, mqttPassword))
      Serial.println("Connected to MQTT server");
    
    else 
    {
      Serial.print("\nFailed state: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// MQTT callback function; handles incoming messages for subscribed topics
void callback(const char* topic, byte* payload, unsigned int length)
{
  // Print received message
  Serial.println("\n-----------------------");
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println("\n-----------------------");

  if (topic == SETTINGS_REQUEST)
  {
    Serial.println("Received 'SETTINGS_REQUEST");
    
    // TODO: Read payload and adjust configuration based on received settings
    
  }
  if (topic == DATA_REQUEST)
  {
    Serial.println("Received DATA_REQUEST");

    // TODO: send data
    
  }
}

