Seville-MQTT
==============================

A sketch for the NodeMCU/ESP8266 which allows controlling a Seville Classics UltraSlimline 40-inch Tower Fan via MQTT.

# Prerequisites
1. NodeMCU or ESP8266.
2. An IR emitter LED.
3. NodeMCU needs to be configured in the Arduino IDE. Detailed instructions are available [here](https://github.com/esp8266/Arduino#installing-with-boards-manager).

# Setup
1. Connect an IR LED to a pin on NodeMCU. The default configuration uses GPIO pin 14 (D5 on NodeMCU) because it is conveniently located right next to a ground.
2. Copy `config.h.example` to `config.h`.
3. Change `config.h` to match your settings.
4. Ensure all required libraries are loaded in the Arduino IDE:
  - [ArduinoJSON](https://github.com/bblanchon/ArduinoJson)
  - [ArduinoOTA](https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA)
  - [IRemoteESP8266](https://github.com/markszabo/IRremoteESP8266)
  - [PubSubClient](https://github.com/knolleary/pubsubclient)
5. Load the sketch.
6. When the sketch starts up it will set the fan to eco speed and turn it off to ensure that the fan is in a clean state.

# Notes
- The red LED flashes everytime a message is sent via MQTT.
- The red LED will stay solid if the board is disconnected from wifi.
- The blue LED will blink anytime IR data is emitted.
- ArduinoOTA is enabled allowing you to upload new versions without needing to plug into a computer. See [the docs](http://esp8266.github.io/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html#arduino-ide) for more information.

# Topics
| Name      | Default State Topic          | Default Command Topic      | Defined as                                     | Accepts                         | Description                                                                                                                                              |
|-----------|------------------------------|----------------------------|------------------------------------------------|---------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|
| Alive     | `esp8266/ac/status`          | N/A                        | `ALIVE_TOPIC`                                  | N/A                             | Contains the current status of the board. `alive` if it's online, otherwise `dead`.                                                                      |
| Power     | `esp8266/ac/on/state`        | `esp8266/ac/on/set`        | `ON_STATE_TOPIC`, `ON_SET_TOPIC`               | Boolean                         | Turns the fan on/off                                                                                                                                     |
| Oscillate | `esp8266/ac/oscillate/state` | `esp8266/ac/oscillate/set` | `OSCILLATE_STATE_TOPIC`, `OSCILLATE_SET_TOPIC` | Boolean                         | Turns oscillation on/off                                                                                                                                 |
| Speed     | `esp8266/ac/speed/state`     | `esp8266/ac/speed/set`     | `SPEED_STATE_TOPIC`, `SPEED_SET_TOPIC`         | `eco`, `low`, `medium`, `high`  | Sets the fan speed                                                                                                                                       |
| Timer     | `esp8266/ac/timer/state`     | `esp8266/ac/timer/set`     | `TIMER_STATE_TOPIC`, `TIMER_SET_TOPIC`         | `00:30` to `07:30` (hh:mm)      | Sets the timer                                                                                                                                           |
| Wind Mode | `esp8266/ac/wind/state`      | `esp8266/ac/wind/set`      | `WIND_STATE_TOPIC`, `WIND_SET_TOPIC`           | `normal`, `sleeping`, `natural` | Sets the "wind mode"                                                                                                                                     |

# Home Assistant Example Configuration
```yaml
mqtt:
  host: 192.168.1.2
  username: username
  password: password
  discovery: true

fan:
  - platform: mqtt
    name: "Seville Fan"
    availability_topic: "esp8266/fan/status"
    command_topic: "esp8266/fan/on/set"
    json_attributes_topic: "esp8266/fan/attributes"
    oscillation_command_topic: "esp8266/fan/oscillate/set"
    oscillation_state_topic: "esp8266/fan/oscillate/state"
    speeds:
      - 'off'
      - eco
      - low
      - medium
      - high
    speed_command_topic: "esp8266/fan/speed/set"
    speed_state_topic: "esp8266/fan/speed/state"
    state_topic: "esp8266/fan/on/state"

input_select:
  seville_fan_timer:
    name: "Seville Fan - Timer"
    icon: mdi:timer
    initial: '0:00'
    options:
      - '0:00'
      - '0:30'
      - '1:00'
      - '1:30'
      - '2:00'
      - '2:30'
      - '3:00'
      - '3:30'
      - '4:00'
      - '4:30'
      - '5:00'
      - '5:30'
      - '6:00'
      - '6:30'
      - '7:00'
      - '7:30'
  seville_fan_wind:
    name: "Seville Fan - Wind Mode"
    icon: mdi:weather-windy
    initial: 'Normal'
    options:
      - 'Normal'
      - 'Sleeping'
      - 'Natural'

automation:
  - alias: Set Fan Wind Mode
    hide_entity: True
    trigger:
      platform: state
      entity_id: input_select.seville_fan_wind
    action:
      - service: mqtt.publish
        data_template:
          topic: "esp8266/fan/wind/set"
          payload_template: '{{ states.input_select.seville_fan_wind.state }}'

  - alias: Set Fan Timer
    hide_entity: True
    trigger:
      platform: state
      entity_id: input_select.seville_fan_timer
    action:
      - service: mqtt.publish
        data_template:
          topic: "esp8266/fan/timer/set"
          payload_template: '{{ states.input_select.seville_fan_timer.state }}'
```

# Debugging
Serial outputs at 115200 baud. You can view the output in the Arduino IDE Serial Monitor.

# Acknowledgements
- This sketch is based on work from [@zeroflow's](https://github.com/zeroflow) [ESPAircon](https://github.com/zeroflow/ESPAircon).
- Initial confirmation that the IR codes I was seeing were correct came from [@scruss's](https://github.com/scruss) [IRTowerFanExample](https://github.com/scruss/IRTowerFanExample).
- I got some great ideas from [@balloob's](https://github.com/balloob) [sketch he made to control his AC unit](https://gist.github.com/balloob/daf310faa80112817d6826fbe5fc399d).

# Contributing
Fork, modify, pull request.

# LICENSE
MIT

# Protocol notes
The IRemoteESP8266 library thinks that the Seville remote is sending NEC codes but whenever I attempted to send NEC codes back they wouldn't work.

The only way I could get this working was to send "raw" IR codes. Quite annoying, since the sketch gets quite a bit bigger with the raw data.

The below notes are from when I was initially documenting the NEC code structure.

[@scruss's](https://github.com/scruss) [IRTowerFanExample](https://github.com/scruss/IRTowerFanExample) has already defined most of these codes, but his work was dated so I need to reconfirm.

You can view a full list of his codes (more complete then what I have below) [here](https://github.com/scruss/IRTowerFanExample/blob/master/IRTowerFan.h).

```
1st digit - Prefix bit (1)
2nd digit - Power Off/On (0/1)
3rd digit - Always 2?
4th digit - Timer in 0.5 hour increments from 0.5 to 7.5 (1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F)
5th digit - Always 3?
6th digit - Oscillate Off/On (0/1)
7th digit - Always 4?
8th digit - Speed (Eco: 3, Low: 0, Medium: 1, High: 2)

Wind is supposedly sending the same speed command twice.

10203043 - Off, eco, no wind, no timer, no oscillate
11203043 - On, eco, no wind, no timer, no oscillate
11203040 - On, low, no wind, no timer, no oscillate
11203041 - On, med, no wind, no timer, no oscillate
11203042 - On, hih, no wind, no timer, no oscillate
11203143 - On, eco, no wind, no timer, oscillate
11203140 - On, low, no wind, no timer, oscillate
11203141 - On, med, no wind, no timer, oscillate
11203142 - On, hih, no wind, no timer, oscillate
11213043 - On, eco, no wind, 0.5 timer, no oscillate
11223043 - On, eco, no wind, 1.0 timer, no oscillate
11233043 - On, eco, no wind, 1.5 timer, no oscillate
11243043 - On, eco, no wind, 2.0 timer, no oscillate
11253043 - On, eco, no wind, 2.5 timer, no oscillate
11263043 - On, eco, no wind, 3.0 timer, no oscillate
11273043 - On, eco, no wind, 3.5 timer, no oscillate
11283043 - On, eco, no wind, 4.0 timer, no oscillate
11293043 - On, eco, no wind, 4.5 timer, no oscillate
112A3043 - On, eco, no wind, 5.0 timer, no oscillate
112B3043 - On, eco, no wind, 5.5 timer, no oscillate
112C3043 - On, eco, no wind, 6.0 timer, no oscillate
112D3043 - On, eco, no wind, 6.5 timer, no oscillate
112E3043 - On, eco, no wind, 7.0 timer, no oscillate
112F3043 - On, eco, no wind, 7.5 timer, no oscillate
```
