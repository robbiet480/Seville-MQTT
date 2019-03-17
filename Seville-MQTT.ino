// Sketch to control a Seville Classics UltraSlimline 40-inch Tower Fan
// Author: Robbie Trencheny
// https://github.com/robbiet480/Seville-MQTT
// License: MIT

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "Seville.h"

#define MQTT_MAX_PACKET_SIZE 384

IRSevilleFan fan(IR_PIN);

void callback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, callback, espClient);

char chipid[20];
char hostname[20];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  payload[length] = '\0';
  String str_payload = String((char*)payload);

  // Set the publishing_payload before we lowercase incoming payload.
  String publishing_topic = "";
  String publishing_payload = String(str_payload).c_str();

  str_payload.toLowerCase();

  String str_topic = String(topic);

  if (str_topic.equals(ON_SET_TOPIC)) {
    bool on = (str_payload == "on");
    fan.setPower(on);
    publishing_topic = ON_STATE_TOPIC;
    if(!on) {
      publishToMQTT(SPEED_STATE_TOPIC, "OFF");
    }
  } else if (str_topic.equals(OSCILLATE_SET_TOPIC)) {
    bool oscillate = (str_payload == "on");
    fan.setOscillation(oscillate);
    publishing_topic = OSCILLATE_STATE_TOPIC;
  } else if (str_topic.equals(SPEED_SET_TOPIC)) {
    if (str_payload == "off") {
      fan.setPower(false);
      publishToMQTT(ON_STATE_TOPIC, "OFF");
    } else if (str_payload == "eco") {
      fan.setSpeed(kSevilleSpeedEco);
    } else if (str_payload == "low") {
      fan.setSpeed(kSevilleSpeedLow);
    } else if (str_payload == "medium") {
      fan.setSpeed(kSevilleSpeedMedium);
    } else if (str_payload == "high") {
      fan.setSpeed(kSevilleSpeedHigh);
    }
    publishing_topic = SPEED_STATE_TOPIC;
  } else if (str_topic.equals(WIND_SET_TOPIC)) {
    if (str_payload == "normal") {
      fan.setWind(kSevilleWindNormal);
    } else if (str_payload == "sleeping") {
      fan.setWind(kSevilleWindSleeping);
    } else if (str_payload == "natural") {
      fan.setWind(kSevilleWindNatural);
    }
    publishing_topic = WIND_STATE_TOPIC;
  } else if (str_topic.equals(TIMER_SET_TOPIC)) {
    if (str_payload == "0:00") {
      fan.setTimer(kSevilleTimerNone);
    } else if (str_payload == "0:30") {
      fan.setTimer(kSevilleTimerHalfHour);
    } else if (str_payload == "1:00") {
      fan.setTimer(kSevilleTimerHour);
    } else if (str_payload == "1:30") {
      fan.setTimer(kSevilleTimerHourAndAHalfHours);
    } else if (str_payload == "2:00") {
      fan.setTimer(kSevilleTimerTwoHours);
    } else if (str_payload == "2:30") {
      fan.setTimer(kSevilleTimerTwoAndAHalfHours);
    } else if (str_payload == "3:00") {
      fan.setTimer(kSevilleTimerThreeHours);
    } else if (str_payload == "3:30") {
      fan.setTimer(kSevilleTimerThreeAndAHalfHours);
    } else if (str_payload == "4:00") {
      fan.setTimer(kSevilleTimerFourHours);
    } else if (str_payload == "4:30") {
      fan.setTimer(kSevilleTimerFourAndAHalfHours);
    } else if (str_payload == "5:00") {
      fan.setTimer(kSevilleTimerFiveHours);
    } else if (str_payload == "5:30") {
      fan.setTimer(kSevilleTimerFiveAndAHalfHours);
    } else if (str_payload == "6:00") {
      fan.setTimer(kSevilleTimerSixHours);
    } else if (str_payload == "6:30") {
      fan.setTimer(kSevilleTimerSixAndAHalfHours);
    } else if (str_payload == "7:00") {
      fan.setTimer(kSevilleTimerSevenHours);
    } else if (str_payload == "7:30") {
      fan.setTimer(kSevilleTimerSevenAndAHalfHours);
    }
    publishing_topic = TIMER_STATE_TOPIC;
  } else {
    Serial.println("No topic matched!");
  }

  fanSend();

  if (publishing_topic != "" && publishing_payload != "") {
    publishToMQTT(publishing_topic.c_str(), publishing_payload.c_str());
  }
}

void setup() {
  sprintf(chipid, "%08X", ESP.getChipId());
  fan.begin();
  Serial.begin(115200);
  Serial.println("Booting");

  pinMode(IR_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  setupWiFi();

  sprintf(hostname, "Seville-MQTT-%s", chipid);

  printState();

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.begin();
}

void setupWiFi() {
  // Disable built in access point
  WiFi.mode(WIFI_STA);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print("state=");
    Serial.println(client.state());

    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, ALIVE_TOPIC, 0, 1, "offline")) {
      Serial.print("Connected to MQTT Broker (");
      Serial.print(MQTT_SERVER);
      Serial.println(")");
      Serial.print("MQTT connection state: ");
      Serial.println(client.state());
      publishToMQTT(ALIVE_TOPIC, "online");
      client.subscribe(ON_SET_TOPIC);
      client.subscribe(OSCILLATE_SET_TOPIC);
      client.subscribe(SPEED_SET_TOPIC);
      client.subscribe(TIMER_SET_TOPIC);
      client.subscribe(WIND_SET_TOPIC);
      publishAttributes();
      publishDiscovery();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    Serial.println("Disconnected from MQTT, starting reconnection!");
    Serial.print("Current WiFi state is: ");
    Serial.println(WiFi.status());

    // Breathing from https://arduining.com/2015/08/20/nodemcu-breathing-led-with-arduino-ide/

    //ramp increasing intensity, Inhalation:
    for (int i=1;i<BRIGHT;i++){
      digitalWrite(RED_LED, LOW);          // turn the LED on.
      delayMicroseconds(i*10);             // wait
      digitalWrite(RED_LED, HIGH);         // turn the LED off.
      delayMicroseconds(PULSE-i*10);       // wait
      delay(0);                            // to prevent watchdog firing.
    }

    //ramp decreasing intensity, Exhalation (half time):
    for (int i=BRIGHT-1;i>0;i--){
      digitalWrite(RED_LED, LOW);          // turn the LED on.
      delayMicroseconds(i*10);             // wait
      digitalWrite(RED_LED, HIGH);         // turn the LED off.
      delayMicroseconds(PULSE-i*10);       // wait
      i--;
      delay(0);                            // to prevent watchdog firing.
    }
    delay(REST);                           // take a rest...
    reconnect();
  }
  ArduinoOTA.handle();
  client.loop();
}

void fanSend() {
  digitalWrite(BLUE_LED, LOW);
  fan.send();
  digitalWrite(BLUE_LED, HIGH);
  printState();
}

void publishToMQTT(const char* topic, const char* payload) {
  digitalWrite(RED_LED, LOW);
  client.publish(topic, payload, true);
  digitalWrite(RED_LED, HIGH);
}

void printState() {
  Serial.println("FAN STATE: ");
  Serial.print("Power: ");
  Serial.println(fan.getPower());
  Serial.print("Timer: ");
  Serial.println(fan.getTimer(), HEX);
  Serial.print("Oscillation: ");
  Serial.println(fan.getOscillation());
  Serial.print("Speed: ");
  Serial.println(fan.getSpeed(), HEX);
  Serial.print("Wind: ");
  Serial.println(fan.getWind(), HEX);
  uint8_t* ir_code = fan.getRaw();
  Serial.print("IR Code: 0x");
  for (uint8_t i = 0; i < kSevilleStateLength; i++)
    Serial.printf("%02X", ir_code[i]);
  Serial.println();

  for (uint8_t ii = 0; ii < kSevilleStateLength; ii++)
    Serial.printf("%02X: %02X\n", ii, ir_code[ii]);
}

void publishAttributes(void) {
  StaticJsonDocument<1024> root;
  root["BSSID"] = WiFi.BSSIDstr();
  root["Chip ID"] = chipid;
  root["Hostname"] = hostname;
  root["IP Address"] = WiFi.localIP().toString();
  root["MAC Address"] = WiFi.macAddress();
  root["MQTT Client ID"] = MQTT_CLIENT_ID;
  root["RSSI"] = WiFi.RSSI();
  root["SSID"] = WiFi.SSID();
  char outgoingJsonBuffer[512];
  serializeJson(root, outgoingJsonBuffer);
  publishToMQTT(HOME_ASSISTANT_ATTRIBUTES_TOPIC, outgoingJsonBuffer);
}

void publishDiscovery(void) {
  StaticJsonDocument<1024> root;
  root["name"] = HOME_ASSISTANT_DISCOVERY_NAME;
  root["availability_topic"] = ALIVE_TOPIC;
  root["json_attributes_topic"] = HOME_ASSISTANT_ATTRIBUTES_TOPIC;
  root["state_topic"] = ON_STATE_TOPIC;
  root["command_topic"] = ON_SET_TOPIC;
  root["oscillation_state_topic"] = OSCILLATE_STATE_TOPIC;
  root["oscillation_command_topic"] = OSCILLATE_SET_TOPIC;
  root["speed_state_topic"] = SPEED_STATE_TOPIC;
  root["speed_command_topic"] = SPEED_SET_TOPIC;
  JsonArray speeds = root.createNestedArray("speeds");
  speeds.add("off");
  speeds.add("eco");
  speeds.add("low");
  speeds.add("medium");
  speeds.add("high");
  char outgoingJsonBuffer[1024];
  serializeJson(root, outgoingJsonBuffer);
  publishToMQTT(HOME_ASSISTANT_MQTT_DISCOVERY_TOPIC, outgoingJsonBuffer);
}
