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
const int pin_DS18B20_TEMP = D4;

// 74HC595 Shift Register pins
const int pin_74HC595_DS = D0;
const int pin_74HC595_STCP = D1;
const int pin_74HC595_SHCP = D2;

const int pin_7SEG_1 = D6; // First 7 segment display
const int pin_7SEG_2 = D7; // Second 7 segment display
const int pin_7SEG_3 = D8; // Third 7 segment display

const int pins_7SEG_ALL[3] = {
  pin_7SEG_1,
  pin_7SEG_2,
  pin_7SEG_3
};

// Byte patterns for 7 seg displays
const byte DIGIT_PATTERN_7SEGMENT[16] =
{
  B00111111,  // 0
  B00000110,  // 1
  B01011011,  // 2
  B01001111,  // 3
  B01100110,  // 4
  B01101101,  // 5
  B01111101,  // 6
  B00000111,  // 7
  B01111111,  // 8
  B01101111,  // 9
  B01110111,  // A
  B01111100,  // b
  B00111001,  // C
  B01011110,  // d
  B01111001,  // E
  B01110001   // F
};

long lastMsgTime = 0;
float oneWireTemp = NULL;

const int delayTime7Seg = 5;

/**
 * Init Libraries
 */
DS18B20 ds(pin_DS18B20_TEMP);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup(void) {
  Serial.begin(115200);
  while (! Serial);

  pinMode(pin_74HC595_DS, OUTPUT);
  pinMode(pin_74HC595_STCP, OUTPUT);
  pinMode(pin_74HC595_SHCP, OUTPUT);  
  pinMode(pin_7SEG_1, OUTPUT);
  pinMode(pin_7SEG_2, OUTPUT);
  pinMode(pin_7SEG_3, OUTPUT);

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
    all7SegmentDisplaysOff();

    // Read temp from sensor
    while (ds.selectNext()) {
      Serial.println("Next..");
      oneWireTemp = ds.getTempC();
      Serial.println(oneWireTemp);
    }

    lastMsgTime = now;

    // Publishing sensor data
    if (oneWireTemp != NULL) {
      mqttPublish(MQTT_TOPIC_TEMPERATURE, oneWireTemp);  
    } else {
      Serial.println("FAILED TO GET TEMP");
    }
    
  }

  // Keep updating the 7 segment displays
  // Break apart a number into 3 digits, with the last being 1 decimal place

  int numOne = 15; // 'F' Char (for error)
  int numTwo = 15; // 'F' Char (for error)
  int numThree = 15; // 'F' Char (for error)
  int numFour = 15; // 'F' Char (for error)

  if (oneWireTemp != NULL) {
    // Get each part of the number
    // We are going for format XX.XX
    // E.g. given the number 12.34
    int lhs = (int)floor(oneWireTemp); // 12
    int rhs = (int)((oneWireTemp - lhs) * 100); // 34
  
    numTwo = lhs % 10; // 2
    numOne = (lhs - numTwo) / 10; // 1
    numFour = rhs % 10; // 4
    numThree = (rhs - numFour) / 10; // 3  
  }
  

  delay(delayTime7Seg);
  update7SegmentDisplay(numOne, 0, false);
  delay(delayTime7Seg);
  update7SegmentDisplay(numTwo, 1, true);
  delay(delayTime7Seg);
  update7SegmentDisplay(numThree, 2, false);
  delay(delayTime7Seg);
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

/*******************************
 * 7 Segment Display functions
 *******************************/
void all7SegmentDisplaysOff() {
  for (int i=0; i<sizeof(pins_7SEG_ALL); i++) {
    digitalWrite(pins_7SEG_ALL[i], LOW);
  }
}
void update7SegmentDisplay(int data, int ledNum, bool showDecimal)
{
  all7SegmentDisplaysOff();
  
  byte pattern;
  
  // get the digit pattern to be updated
  pattern = DIGIT_PATTERN_7SEGMENT[data];

  if (showDecimal) {
    pattern = pattern | B10000000;
  }

  // turn off the output of 74HC595
  digitalWrite(pin_74HC595_STCP, LOW);
  
  // update data pattern to be outputed from 74HC595
  // because it's a common anode LED, the pattern needs to be inverted
  // shiftOut(pin_74HC595_DS, pin_74HC595_SHCP, MSBFIRST, ~pattern); // Common Anode
  shiftOut(pin_74HC595_DS, pin_74HC595_SHCP, MSBFIRST, pattern); // Common Cathode
  
  // turn on the output of 74HC595
  digitalWrite(pin_74HC595_STCP, HIGH);
  digitalWrite(pins_7SEG_ALL[ledNum], HIGH);
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
