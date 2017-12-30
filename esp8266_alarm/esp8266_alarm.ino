#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "WiFi.h"

extern "C" {
#include "gpio.h"
#include "user_interface.h"
}

#define DEBUG true
#define VERBOSE false

// SSID and PASSWORD will be imported from WiFi.h
// TODO 2: update the values below
const char* MQTT_CLIENT = "ESP8266_Alarm_front";
const char* MQTT_IP = "192.168.0.10";
const int   MQTT_PORT = 1883;
const char* MQTT_TOPIC = "mqtt";
// TODO dev1: use user and passwor for mqtt connection
const char* MQTT_USER = "";
const char* MQTT_PASSWORD = "";
const char* MQTT_MESSAGE = "{\"alarm\": {\"status\":\"activated\", \"door\":\"front\"}}";

// Objet that helps estblish a connection to MQTT server - https://techtutorialsx.com/2017/04/09/esp8266-connecting-to-mqtt-broker/ 
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  if (DEBUG)
    Serial.begin(115200);
  
  pinMode(2, INPUT); // this pin is connected to the PIR sensor.

  if (DEBUG) {
    Serial.print("Connecting to SSID: ");
    Serial.println(SSID);
  }
  // Set wifi in station mode with static ip
  WiFi.mode(WIFI_STA);
  WiFi.config(IP, GATEWAY, SUBNET);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (DEBUG) 
      Serial.print(".");
  }
  delay(1000);
  if (DEBUG) {
    Serial.println("");
    Serial.print("WiFi connected with static IP: ");
    Serial.println(IP);
  }
  
  if (DEBUG) 
    Serial.println("Connecting to MQTT");
  client.setServer(MQTT_IP, MQTT_PORT);
  
  if (VERBOSE) 
    Serial.println("Setup finished");
}

void connect_to_mqtt() {
  if (VERBOSE) 
    Serial.println("#connect_to_mqtt, begin");
  while (!client.connected()) {
    if (DEBUG) 
      Serial.print("Connecting to MQTT server");
    if (client.connect(MQTT_CLIENT)) {
      if (VERBOSE) 
        Serial.println("connected");
    } else {
      if (DEBUG) {
        Serial.print("MQTT connection failed, current state code = ");
        Serial.print(client.state());
        Serial.println("; try again in 7 seconds");
      }
      delay(7000);
    }
  }
  if (VERBOSE) 
    Serial.println("#connect_to_mqtt, end");
}

void loop() {
  if (VERBOSE) 
    Serial.println("#loop, begin");
  // publish alarm message
  if (!client.connected()) {
    connect_to_mqtt();
  }
  client.loop();
  if (DEBUG) {
    Serial.print("Publish message: ");
    Serial.println(MQTT_MESSAGE);
    Serial.print("On topic: ");
    Serial.println(MQTT_TOPIC);
  }
  client.publish(MQTT_TOPIC, MQTT_MESSAGE);
  
  if (DEBUG) 
    Serial.println("Going to light sleep in 1 second");
  delay(1000);
  
  sleepNow();
}

void sleepNow() {
  if (VERBOSE) 
    Serial.println("#sleepNow, begin");
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  gpio_pin_wakeup_enable(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL); // trigger on low level of pin #2
  if (VERBOSE) 
    Serial.println("#sleepNow, after gpio_pin_wakeup_enable()");
  wifi_fpm_open();
  delay(100);
  wifi_fpm_set_wakeup_cb(wakeUp);
  wifi_fpm_do_sleep(0xFFFFFFF); 
  delay(100);
  if (VERBOSE) 
    Serial.println("#sleepNow, end");
}

void wakeUp(void) {
  if (VERBOSE) 
    Serial.println("#wakeUp, begin");
  wifi_fpm_close;
  wifi_set_opmode(STATION_MODE);
  if (VERBOSE) 
    Serial.println("#wakeUp, before wifi_station_connect() ");
  wifi_station_connect();
  if (VERBOSE) 
    Serial.println("#wakeUp, end");
}

