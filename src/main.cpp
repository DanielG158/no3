#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <BH1750.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* ssid = "ASUS";
const char* password = "password";

// MQTT credentials and settings
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_username = "dan";
const char* mqtt_password = "ger";
const char* mqtt_subscribe_topic = "cobad";
const char* mqtt_publisher_topic = "sensor_data";
WiFiClient espClient;
PubSubClient client(espClient);

// Pin definitions
const int LIGHT_SENSOR_ADDRESS = 0x23; // I2C address of light sensor
const int LED_PIN = 13; //  LED and buzzer
// Sensor type definitions
const int LIGHT_SENSOR_TYPE = BH1750::ONE_TIME_HIGH_RES_MODE; // 1 lx resolution

// Sensor objects
BH1750 lightMeter; // I2C address 0x23

void setup() {
  Serial.begin(115200); // Initialize serial communication
  Wire.begin(21, 22); // SDA, SCL
  lightMeter.begin(); // Initialize light sensor
  delay(100);  // Wait for sensor to initialize
  pinMode(LED_PIN, OUTPUT); // Initialize LED pin as output
  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

}

void loop() {
  static unsigned long lastLightTime = 0; //this static variable is for the light to be read every 2 seconds

  // Take light reading every 2 seconds
  if (millis() - lastLightTime >= 2000) {
    uint16_t lux = lightMeter.readLightLevel();  //to make the BH1750 read the light level
    if (!isnan(lux)) { //if the light level is not a number, print the light level
      Serial.print("Light: ");
      Serial.print(lux);
      Serial.println(" lux");
      //digitalWrite(LED_PIN, HIGH);
if (lux > 400) {
        // Turn on buzzer if light level is greater than 400 lux
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }

      // Publish light sensor data to MQTT broker
      if (!isnan(lux) && lux > 400) {
        char buffer[10];
      snprintf(buffer, 10, "%d", lux);
      client.publish(mqtt_publisher_topic, ("The light sensor reading is " + String(buffer) + " lux").c_str());
    } else {
      Serial.println("Kotak isn't opened! Thank god :)"); //if the light level is a number, print an error message
      //digitalWrite(LED_PIN, LOW); // Turn off LED
    }
    lastLightTime = millis(); //this is to make the light level read every 2 seconds
  }
}

  /*  char buffer[10];
      snprintf(buffer, 10, "%d", lux);
      client.publish(mqtt_publisher_topic, ("The light sensor reading is " + String(buffer) + " lux").c_str());
    } else {
      Serial.println("Error reading light level!"); //if the light level is a number, print an error message
    }
    lastLightTime = millis(); //this is to make the light level read every 2 seconds
  }*/

  // Maintain MQTT connection
  if (!client.connected()) {
    client.connect("ESP32Client", mqtt_username, mqtt_password);
  }
  client.loop();

  delay(10);
}
