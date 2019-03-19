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
char chipid[20];
char localIP[16];
int startupCompleted = 0;

unsigned long sendTimer;
bool waitingToSend = false;

bool are_equal(char* a, char* b) {
  return strcmp(a, b)==0;
}

bool are_equal(byte* a, char* b) {
  return are_equal((char *)a, b);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Since messages are retained, this logic skips the callback
  // for those until all have been processed.
  if(startupCompleted < 5) {
    startupCompleted += 1;
    return;
  }

  payload[length] = '\0';

  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.

  // Allocate the correct amount of memory for the payload copy
  byte* copied_payload = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(copied_payload, payload, length);

  Serial.printf("Message arrived [%s]: %s\n", topic, payload);

  char publishing_topic[64];

  if (are_equal(topic, ON_SET_TOPIC)) {
    bool on = are_equal(payload, POWER_ON_PAYLOAD);
    fan.setPower(on);
    strcpy(publishing_topic, ON_STATE_TOPIC);
    mqttClient.publish(SPEED_STATE_TOPIC, mapSpeedVal(), true);
  } else if (are_equal(topic, OSCILLATE_SET_TOPIC)) {
    bool oscillate = are_equal(payload, OSCILLATION_ON_PAYLOAD);
    fan.setOscillation(oscillate);
    strcpy(publishing_topic, OSCILLATE_STATE_TOPIC);
  } else if (are_equal(topic, SPEED_SET_TOPIC)) {
    if (are_equal(payload, SPEED_OFF_PAYLOAD)) {
      fan.setPower(false);
      mqttClient.publish(ON_STATE_TOPIC, POWER_OFF_PAYLOAD, true);
    } else if (are_equal(payload, SPEED_ECO_PAYLOAD)) {
      fan.setSpeed(kSevilleSpeedEco);
    } else if (are_equal(payload, SPEED_LOW_PAYLOAD)) {
      fan.setSpeed(kSevilleSpeedLow);
    } else if (are_equal(payload, SPEED_MEDIUM_PAYLOAD)) {
      fan.setSpeed(kSevilleSpeedMedium);
    } else if (are_equal(payload, SPEED_HIGH_PAYLOAD)) {
      fan.setSpeed(kSevilleSpeedHigh);
    } else {
      Serial.printf("Unknown speed: %s!", payload);
    }
    strcpy(publishing_topic, SPEED_STATE_TOPIC);
  } else if (are_equal(topic, WIND_SET_TOPIC)) {
    if (are_equal(payload, WIND_NORMAL_PAYLOAD)) {
      fan.setWind(kSevilleWindNormal);
    } else if (are_equal(payload, WIND_SLEEPING_PAYLOAD)) {
      fan.setWind(kSevilleWindSleeping);
    } else if (are_equal(payload, WIND_NATURAL_PAYLOAD)) {
      fan.setWind(kSevilleWindNatural);
    } else {
      Serial.printf("Unknown wind: %s!", payload);
    }
    strcpy(publishing_topic, WIND_STATE_TOPIC);
  } else if (are_equal(topic, TIMER_SET_TOPIC)) {
    if (are_equal(payload, TIMER_NONE_PAYLOAD)) {
      fan.setTimer(kSevilleTimerNone);
    } else if (are_equal(payload, TIMER_HALF_HOUR_PAYLOAD)) {
      fan.setTimer(kSevilleTimerHalfHour);
    } else if (are_equal(payload, TIMER_HOUR_PAYLOAD)) {
      fan.setTimer(kSevilleTimerHour);
    } else if (are_equal(payload, TIMER_HOUR_AND_A_HALF_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerHourAndAHalfHours);
    } else if (are_equal(payload, TIMER_TWO_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerTwoHours);
    } else if (are_equal(payload, TIMER_TWO_AND_A_HALF_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerTwoAndAHalfHours);
    } else if (are_equal(payload, TIMER_THREE_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerThreeHours);
    } else if (are_equal(payload, TIMER_THREE_AND_A_HALF_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerThreeAndAHalfHours);
    } else if (are_equal(payload, TIMER_FOUR_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerFourHours);
    } else if (are_equal(payload, TIMER_FOUR_AND_A_HALF_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerFourAndAHalfHours);
    } else if (are_equal(payload, TIMER_FIVE_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerFiveHours);
    } else if (are_equal(payload, TIMER_FIVE_AND_A_HALF_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerFiveAndAHalfHours);
    } else if (are_equal(payload, TIMER_SIX_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerSixHours);
    } else if (are_equal(payload, TIMER_SIX_AND_A_HALF_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerSixAndAHalfHours);
    } else if (are_equal(payload, TIMER_SEVEN_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerSevenHours);
    } else if (are_equal(payload, TIMER_SEVEN_AND_A_HALF_HOURS_PAYLOAD)) {
      fan.setTimer(kSevilleTimerSevenAndAHalfHours);
    } else {
      Serial.printf("Unknown timer: %s!", payload);
    }
    strcpy(publishing_topic, TIMER_STATE_TOPIC);
  } else {
    Serial.println("No topic matched!");
  }

  sendTimer = millis();
  waitingToSend = true;

  if (!are_equal(publishing_topic, "")) {
    digitalWrite(RED_LED, LOW);
    mqttClient.publish(publishing_topic, copied_payload, true);
    digitalWrite(RED_LED, HIGH);
  }

  free(copied_payload);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  pinMode(IR_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  sprintf(chipid, "%08X", ESP.getChipId());
  sprintf(hostname, "Seville-MQTT-%08X", ESP.getChipId());

  fan.begin();

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

  sprintf(localIP, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.printf("IP address: %s\n", localIP);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.printf("state=%s\n", mqttClientState());

    // Attempt to connect
    if (mqttClient.connect(hostname, MQTT_USER, MQTT_PASS, ALIVE_TOPIC, MQTT_QOS, 1, OFFLINE_PAYLOAD)) {
      Serial.printf("Connected to MQTT Broker (%s)\n", MQTT_SERVER);
      Serial.printf("MQTT connection state: %s\n", mqttClientState());

      fan.reset();
      fan.send();

      waitingToSend = false;
      sendTimer = 0;

      // Set all the default values on the topics
      mqttClient.publish(ALIVE_TOPIC, ONLINE_PAYLOAD, true);

      mqttClient.publish(ON_SET_TOPIC, POWER_OFF_PAYLOAD, false);
      mqttClient.publish(OSCILLATE_SET_TOPIC, OSCILLATION_OFF_PAYLOAD, false);
      mqttClient.publish(SPEED_SET_TOPIC, SPEED_ECO_PAYLOAD, false);
      mqttClient.publish(TIMER_SET_TOPIC, TIMER_NONE_PAYLOAD, false);
      mqttClient.publish(WIND_SET_TOPIC, WIND_NORMAL_PAYLOAD, false);

      mqttClient.publish(ON_STATE_TOPIC, POWER_OFF_PAYLOAD, true);
      mqttClient.publish(OSCILLATE_STATE_TOPIC, OSCILLATION_OFF_PAYLOAD, true);
      mqttClient.publish(SPEED_STATE_TOPIC, SPEED_ECO_PAYLOAD, true);
      mqttClient.publish(TIMER_STATE_TOPIC, TIMER_NONE_PAYLOAD, true);
      mqttClient.publish(WIND_STATE_TOPIC, WIND_NORMAL_PAYLOAD, true);

      // Subscribe to all topics
      mqttClient.subscribe(ON_SET_TOPIC);
      mqttClient.subscribe(OSCILLATE_SET_TOPIC);
      mqttClient.subscribe(SPEED_SET_TOPIC);
      mqttClient.subscribe(TIMER_SET_TOPIC);
      mqttClient.subscribe(WIND_SET_TOPIC);

      publishAttributes();
      publishDiscovery();
    } else {
      Serial.printf("failed, rc=%s try again in 5 seconds\n", mqttClientState());
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
  if(waitingToSend && (millis()-sendTimer >= 250UL)) {
    waitingToSend = false;
    sendTimer = 0;
    Serial.println("Flushing pending commands to IR!");
    digitalWrite(BLUE_LED, LOW);
    fan.send();
    digitalWrite(BLUE_LED, HIGH);
    printState();
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

void printState() {
  uint8_t* ir_code = fan.getRaw();
  char ir_code_str[24];
  sprintf(ir_code_str, "%02X %02X %02X %02X %02X %02X %02X %02X",
          ir_code[0], ir_code[1], ir_code[2], ir_code[3], ir_code[4], ir_code[5], ir_code[6], ir_code[7]);

  Serial.printf("New Fan State: Power: %s, Timer: %s, Oscillation: %s, Speed: %s, Wind: %s, IR Code: 0x %s\n",
                 fan.getPowerString(), fan.getTimerString(), fan.getOscillationString(),
                 fan.getSpeedString(), fan.getWindString(), ir_code_str);
}

void publishAttributes() {
  StaticJsonDocument<512> root;
  root["BSSID"] = WiFi.BSSIDstr();
  root["Chip ID"] = chipid;
  root["Hostname"] = hostname;
  root["IP Address"] = localIP;
  root["MAC Address"] = WiFi.macAddress();
  root["RSSI"] = WiFi.RSSI();
  root["SSID"] = WiFi.SSID();
  char outgoingJsonBuffer[512];
  serializeJson(root, outgoingJsonBuffer);
  mqttClient.publish(HOME_ASSISTANT_ATTRIBUTES_TOPIC, outgoingJsonBuffer, true);
}

void publishDiscovery() {
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
  root["unique_id"] = chipid;
  JsonArray speeds = root.createNestedArray("speeds");
  speeds.add(SPEED_OFF_PAYLOAD);
  speeds.add(SPEED_ECO_PAYLOAD);
  speeds.add(SPEED_LOW_PAYLOAD);
  speeds.add(SPEED_MEDIUM_PAYLOAD);
  speeds.add(SPEED_HIGH_PAYLOAD);
  char outgoingJsonBuffer[768];
  serializeJson(root, outgoingJsonBuffer);
  mqttClient.publish(HOME_ASSISTANT_MQTT_DISCOVERY_TOPIC, outgoingJsonBuffer, true);
}

char* mqttClientState() {
  switch(mqttClient.state()) {
    case MQTT_CONNECTION_TIMEOUT:
      return "Connection Timeout (code: -4)";
    case MQTT_CONNECTION_LOST:
      return "Connection Lost (code: -3)";
    case MQTT_CONNECT_FAILED:
      return "Connect Failed (code: -2)";
    case MQTT_DISCONNECTED:
      return "Disconnected (code: -1)";
    case MQTT_CONNECTED:
      return "Connected (code: 0)";
    case MQTT_CONNECT_BAD_PROTOCOL:
      return "Connect Bad Protocol (code: 1)";
    case MQTT_CONNECT_BAD_CLIENT_ID:
      return "Connect Bad Client Id (code: 2)";
    case MQTT_CONNECT_UNAVAILABLE:
      return "Connect Unavailable (code: 3)";
    case MQTT_CONNECT_BAD_CREDENTIALS:
      return "Connect Bad Credentials (code: 4)";
    case MQTT_CONNECT_UNAUTHORIZED:
      return "Connect Unauthorized (code: 5)";
    default:
      return "Unknown";
  }
}

char* mapSpeedVal() {
  if(!fan.getPower()) {
    return SPEED_OFF_PAYLOAD;
  }
  switch(fan.getSpeed()) {
    case kSevilleSpeedEco:
      return SPEED_ECO_PAYLOAD;
    case kSevilleSpeedLow:
      return SPEED_LOW_PAYLOAD;
    case kSevilleSpeedMedium:
      return SPEED_MEDIUM_PAYLOAD;
    case kSevilleSpeedHigh:
      return SPEED_HIGH_PAYLOAD;
  }
}
