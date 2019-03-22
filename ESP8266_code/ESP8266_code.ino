#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <BH1750FVI.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <inttypes.h>

// Thresholds
#define LUMOSITY_DELTA_THRESHOLD 50
#define RBG_DELTA_THRESHOLD_LOW  0x3fffffff
#define RGB_DELTA_THRESHOLD_HIGH 0xbffffffd

// I2C addresses
#define BH_I2C_ADDR  0x23 // I2C bus address of BH sensor
#define TCS_I2C_ADDR 0x29 // I2C bus address of TCS sensor

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
const char* mqttUser = "";  // no username
const char* mqttPassword = ""; // no password

WiFiClient wifiClient;
PubSubClient client(wifiClient);

BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

// Sensor settings
const char* sensorName;
boolean indoor;
boolean emitOnChange;

// Tracking current and previous lumosity values for threshold calculations
int curr_lux, prev_lux;
int R, G, B, C;

void setup()
{
  curr_lux = 0;
  prev_lux = 0;
  R = 0;
  G = 0;
  B = 0;
  C = 0;

  delay(100);
  Serial.begin(115200);
  delay(100);

  // Initialize BH1750 sensor
  LightSensor.begin();
  

  // Connect to WiFi network
  wifi_connect();

  // TODO: Initialize TCS sensor
  Wire.begin();
  Wire.beginTransmission(TCS_I2C_ADDR);
  // Enable device (interrupt, wait, RGBC, and internal oscillator)
  Wire.write(0x80); // Command to write to ENABLE register
  Wire.write(0x1B); // This data is written to the ENABLE register to enable device
  Wire.endTransmission(TCS_I2C_ADDR);
  delay(200);

  // Verify that TCS status says RGBC initialized
  Wire.beginTransmission(TCS_I2C_ADDR);
  Wire.write(0x92); // Command to read from ID register
  Wire.endTransmission(TCS_I2C_ADDR);
  Wire.requestFrom(TCS_I2C_ADDR, 1);
  byte b1 = Wire.read();
  Serial.print("First byte read: ");
  Serial.println(b1, HEX);
  
  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  mqtt_connect();

  // Subscribe to JOIN 
  if (client.subscribe(JOIN)) 
    Serial.println("Subscribed to JOIN");

  // Publish JSON-formatted message containing UUID to JOIN
  StaticJsonDocument<200> doc;
  doc["uuid"] = UUID;
  char msg[200];
  int bytes_written = serializeJson(doc, msg);
    
  if (bytes_written == 0)
  {
    Serial.print("serializeJson() failed, bytes written: ");
    Serial.println(bytes_written);
  }
  else
  {
    // If serialization succeeded, publish message to JOIN
    if (client.publish(JOIN, msg)) 
      Serial.println("Published device UUID to JOIN"); 
  }
  
  // Subscribe to DATA 
  if (client.subscribe(DATA)) 
    Serial.println("Subscribed to DATA");

  // Subscribe to SETTINGS_SET
  if (client.subscribe(SETTINGS_SET)) 
    Serial.println("Subscribed to SETTINGS_SET");

  // Subscribe to SETTINGS_REQUEST
  if (client.subscribe(SETTINGS_REQUEST)) 
    Serial.println("Subscribed to SETTINGS_REQUEST");

  // Subscribe to SETTINGS
  if (client.subscribe(SETTINGS)) 
    Serial.println("Subscribed to SETTINGS");

  // Subscribe to DATA_REQUEST
  if (client.subscribe(DATA_REQUEST)) 
    Serial.println("Subscribed to DATA_REQUEST");
}

void loop()
{
  curr_lux = LightSensor.GetLightIntensity();

  // TODO: Read RGB sensor here

  // Significant change in light intensity, publish update to DATA
  if (abs(curr_lux - prev_lux) >= LUMOSITY_DELTA_THRESHOLD)
  {
    publish_data(curr_lux, 0, 0, 0);
  }
  
  if (!client.loop())
  {
    // Try to reconnect to MQTT server
    Serial.println("Lost connection to the mqtt server");
    mqtt_connect();
  }
  
  prev_lux = curr_lux;  // Update previous lux values for threshold tracking
}

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

  if (topic == JOIN)
  {
    Serial.println("Received acknowledgement from broker; UUID has been published");
  }
  
  if (topic == SETTINGS_SET) // Deserialize message and populate sensor settings
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    }
    else
    {      
        // If deserialization succeeded, populate sensor settings from payload
        sensorName = doc["name"];
        indoor = doc["indoor"];
        emitOnChange = doc["emitOnChange"];
    }
  }
  if (topic == SETTINGS_REQUEST)
  {
    Serial.println("Received 'SETTINGS_REQUEST");
    
    // TODO: Publish settings data to SETTINGS topic
    
    
  }
  if (topic == DATA_REQUEST)
  {
    Serial.println("Received DATA_REQUEST");
    // Publish lumosity and RGB data to DATA topic
    publish_data(LightSensor.GetLightIntensity(), 0, 0, 0);

  }  
}

void publish_data(int lux, int R, int G, int B)
{
  // TODO: Package data in JSON format per MQTT Ambiance topic design 
  char payload[256];
  sprintf(payload, "%d", lux);
  if (!client.publish(DATA, payload))
    Serial.println("Failed to publish new lux reading to DATA");
}
