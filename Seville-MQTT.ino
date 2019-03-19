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

#define MQTT_MAX_PACKET_SIZE 768

IRSevilleFan fan(IR_PIN);

void mqttCallback(char* topic, byte* payload, unsigned int length);

WiFiClient espClient;
PubSubClient mqttClient(MQTT_SERVER, MQTT_PORT, mqttCallback, espClient);

char hostname[20];

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  payload[length] = '\0';
  String str_payload = String((char*)payload);

  String publishing_topic = "";
  String publishing_payload = String(str_payload).c_str();

  String str_topic = String(topic);

  if (str_topic.equals(ON_SET_TOPIC)) {
    bool on = (str_payload == POWER_ON_PAYLOAD);
    fan.setPower(on);
    publishing_topic = ON_STATE_TOPIC;
    if(!on) {
      publishToMQTT(SPEED_STATE_TOPIC, SPEED_OFF_PAYLOAD);
    }
  } else if (str_topic.equals(OSCILLATE_SET_TOPIC)) {
    bool oscillate = (str_payload == OSCILLATION_ON_PAYLOAD);
    fan.setOscillation(oscillate);
    publishing_topic = OSCILLATE_STATE_TOPIC;
  } else if (str_topic.equals(SPEED_SET_TOPIC)) {
    if (str_payload == SPEED_OFF_PAYLOAD) {
      fan.setPower(false);
      publishToMQTT(ON_STATE_TOPIC, POWER_OFF_PAYLOAD);
    } else if (str_payload == SPEED_ECO_PAYLOAD) {
      fan.setSpeed(kSevilleSpeedEco);
    } else if (str_payload == SPEED_LOW_PAYLOAD) {
      fan.setSpeed(kSevilleSpeedLow);
    } else if (str_payload == SPEED_MEDIUM_PAYLOAD) {
      fan.setSpeed(kSevilleSpeedMedium);
    } else if (str_payload == SPEED_HIGH_PAYLOAD) {
      fan.setSpeed(kSevilleSpeedHigh);
    }
    publishing_topic = SPEED_STATE_TOPIC;
  } else if (str_topic.equals(WIND_SET_TOPIC)) {
    if (str_payload == WIND_NORMAL_PAYLOAD) {
      fan.setWind(kSevilleWindNormal);
    } else if (str_payload == WIND_SLEEPING_PAYLOAD) {
      fan.setWind(kSevilleWindSleeping);
    } else if (str_payload == WIND_NATURAL_PAYLOAD) {
      fan.setWind(kSevilleWindNatural);
    }
    publishing_topic = WIND_STATE_TOPIC;
  } else if (str_topic.equals(TIMER_SET_TOPIC)) {
    if (str_payload == TIMER_NONE_PAYLOAD) {
      fan.setTimer(kSevilleTimerNone);
    } else if (str_payload == TIMER_HALF_HOUR_PAYLOAD) {
      fan.setTimer(kSevilleTimerHalfHour);
    } else if (str_payload == TIMER_HOUR_PAYLOAD) {
      fan.setTimer(kSevilleTimerHour);
    } else if (str_payload == TIMER_HOUR_AND_A_HALF_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerHourAndAHalfHours);
    } else if (str_payload == TIMER_TWO_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerTwoHours);
    } else if (str_payload == TIMER_TWO_AND_A_HALF_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerTwoAndAHalfHours);
    } else if (str_payload == TIMER_THREE_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerThreeHours);
    } else if (str_payload == TIMER_THREE_AND_A_HALF_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerThreeAndAHalfHours);
    } else if (str_payload == TIMER_FOUR_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerFourHours);
    } else if (str_payload == TIMER_FOUR_AND_A_HALF_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerFourAndAHalfHours);
    } else if (str_payload == TIMER_FIVE_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerFiveHours);
    } else if (str_payload == TIMER_FIVE_AND_A_HALF_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerFiveAndAHalfHours);
    } else if (str_payload == TIMER_SIX_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerSixHours);
    } else if (str_payload == TIMER_SIX_AND_A_HALF_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerSixAndAHalfHours);
    } else if (str_payload == TIMER_SEVEN_HOURS_PAYLOAD) {
      fan.setTimer(kSevilleTimerSevenHours);
    } else if (str_payload == TIMER_SEVEN_AND_A_HALF_HOURS_PAYLOAD) {
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
  Serial.begin(115200);
  Serial.println("Booting");

  pinMode(IR_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  sprintf(hostname, "Seville-MQTT-%08X", ESP.getChipId());

  setupWiFi();

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.begin();
}

void setupWiFi() {
  // Disable built in access point
  WiFi.mode(WIFI_STA);

  WiFi.hostname(hostname);

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
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.print("state=");
    Serial.println(mqttClient.state());

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, ALIVE_TOPIC, MQTT_QOS, 1, OFFLINE_PAYLOAD)) {
      fan.begin();
      printState();
      fanSend();
      Serial.print("Connected to MQTT Broker (");
      Serial.print(MQTT_SERVER);
      Serial.println(")");
      Serial.print("MQTT connection state: ");
      Serial.println(mqttClient.state());
      publishToMQTT(ALIVE_TOPIC, ONLINE_PAYLOAD);

      // Subscribe to all topics
      mqttClient.subscribe(ON_SET_TOPIC);
      mqttClient.subscribe(OSCILLATE_SET_TOPIC);
      mqttClient.subscribe(SPEED_SET_TOPIC);
      mqttClient.subscribe(TIMER_SET_TOPIC);
      mqttClient.subscribe(WIND_SET_TOPIC);
      publishAttributes();
      publishDiscovery();
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
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
  mqttClient.loop();
}

void fanSend() {
  digitalWrite(BLUE_LED, LOW);
  fan.send();
  digitalWrite(BLUE_LED, HIGH);
  printState();
}

void publishToMQTT(const char* topic, const char* payload) {
  digitalWrite(RED_LED, LOW);
  mqttClient.publish(topic, payload, true);
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
    Serial.printf(" %02X", ir_code[i]);
  Serial.println();
}

void publishAttributes(void) {
  StaticJsonDocument<512> root;
  root["BSSID"] = WiFi.BSSIDstr();
  root["Chip ID"] = String(ESP.getChipId(), HEX);
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
  StaticJsonDocument<768> root;
  root["availability_topic"] = ALIVE_TOPIC;
  root["command_topic"] = ON_SET_TOPIC;
  root["json_attributes_topic"] = HOME_ASSISTANT_ATTRIBUTES_TOPIC;
  root["name"] = HOME_ASSISTANT_DISCOVERY_NAME;
  root["oscillation_command_topic"] = OSCILLATE_SET_TOPIC;
  root["oscillation_state_topic"] = OSCILLATE_STATE_TOPIC;
  root["payload_available"] = ONLINE_PAYLOAD;
  root["payload_high_speed"] = SPEED_HIGH_PAYLOAD;
  root["payload_low_speed"] = SPEED_LOW_PAYLOAD;
  root["payload_medium_speed"] = SPEED_MEDIUM_PAYLOAD;
  root["payload_not_available"] = OFFLINE_PAYLOAD;
  root["payload_off"] = POWER_OFF_PAYLOAD;
  root["payload_on"] = POWER_ON_PAYLOAD;
  root["payload_oscillation_off"] = OSCILLATION_OFF_PAYLOAD;
  root["payload_oscillation_on"] = OSCILLATION_ON_PAYLOAD;
  root["speed_command_topic"] = SPEED_SET_TOPIC;
  root["speed_state_topic"] = SPEED_STATE_TOPIC;
  root["state_topic"] = ON_STATE_TOPIC;
  root["unique_id"] = String(ESP.getChipId(), HEX);
  JsonArray speeds = root.createNestedArray("speeds");
  speeds.add(SPEED_OFF_PAYLOAD);
  speeds.add(SPEED_ECO_PAYLOAD);
  speeds.add(SPEED_LOW_PAYLOAD);
  speeds.add(SPEED_MEDIUM_PAYLOAD);
  speeds.add(SPEED_HIGH_PAYLOAD);
  char outgoingJsonBuffer[768];
  serializeJson(root, outgoingJsonBuffer);
  publishToMQTT(HOME_ASSISTANT_MQTT_DISCOVERY_TOPIC, outgoingJsonBuffer);
}
