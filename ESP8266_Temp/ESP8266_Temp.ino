/**
 * Useful Links
 * 
 * DS18B20 (XC3700) Temp Sensor: https://www.jaycar.com.au/medias/sys_master/images/9346271707166/XC3700-manualMain.pdf 
 * 
 * Required Libraries:
 *  - DS18B20
 *  - ESP8266WiFi
 *  - PubSubClient
 *  - Copy the constants.example.h file to constants.h and enter details.
 *  
 * Pins:
 * D4: Temp Sensor (DS18B20)
 */

#include <DS18B20.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "constants.h"



/**
 * Pin Definitions
 */
const int pin_DS18B20_TEMP = D3;

long lastMsgTime = 0;

/**
 * Init Libraries
 */
DS18B20 ds(pin_DS18B20_TEMP);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup(void) {
  Serial.begin(115200);
  while (! Serial);

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
}

void loop(void) {

  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsgTime > MQTT_PUBLISH_DELAY) {

    // Read temp from sensor
    float oneWireTemp = NULL;

    while (ds.selectNext()) {
      Serial.println("Next..");
      oneWireTemp = ds.getTempC();
      Serial.println(oneWireTemp);
    }

    lastMsgTime = now;

    // Publishing sensor data
    mqttPublish(MQTT_TOPIC_TEMPERATURE, oneWireTemp);
  }
}

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");

      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_STATE, "connected", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void mqttPublish(char *topic, float payload) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic, String(payload).c_str(), true);
}
