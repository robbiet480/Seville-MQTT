// Seville Classics Ultra Slimline Fan
//
// Copyright 2019 Robbie Trencheny (@robbie)

#ifndef SEVILLE_H_
#define SEVILLE_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"

// Constants
const uint16_t kSevilleBits                   = 64;
const uint16_t kSevilleDefaultRepeat          = kNoRepeat;
const uint16_t kSevilleStateLength            = 8;

const uint16_t kSevilleHdrMark                = 9350;
const uint16_t kSevilleHdrSpace               = 4500;
const uint16_t kSevilleBitMark                = 590;
const uint16_t kSevilleOneSpace               = 1671;
const uint16_t kSevilleZeroSpace              = 546;
const uint16_t kSevilleMsgSpace               = 10000;

// Digit 2, Byte 1 - Power
const uint8_t kSevillePowerOn                 = 0x11;
const uint8_t kSevillePowerOff                = 0x10;

// Digit 4, Byte 3 - Timer
const uint8_t kSevilleTimerNone               = 0x20;
const uint8_t kSevilleTimerHalfHour           = 0x21;
const uint8_t kSevilleTimerHour               = 0x22;
const uint8_t kSevilleTimerHourAndAHalfHours  = 0x23;
const uint8_t kSevilleTimerTwoHours           = 0x24;
const uint8_t kSevilleTimerTwoAndAHalfHours   = 0x25;
const uint8_t kSevilleTimerThreeHours         = 0x26;
const uint8_t kSevilleTimerThreeAndAHalfHours = 0x27;
const uint8_t kSevilleTimerFourHours          = 0x28;
const uint8_t kSevilleTimerFourAndAHalfHours  = 0x29;
const uint8_t kSevilleTimerFiveHours          = 0x2A;
const uint8_t kSevilleTimerFiveAndAHalfHours  = 0x2B;
const uint8_t kSevilleTimerSixHours           = 0x2C;
const uint8_t kSevilleTimerSixAndAHalfHours   = 0x2D;
const uint8_t kSevilleTimerSevenHours         = 0x2E;
const uint8_t kSevilleTimerSevenAndAHalfHours = 0x2F;

// Digit 6, Byte 5 - Oscillation
const uint8_t kSevilleOscillationOn           = 0x31;
const uint8_t kSevilleOscillationOff          = 0x30;

// Digit 8, Byte 7 - Speed
const uint8_t kSevilleSpeedEco                = 0x43;
const uint8_t kSevilleSpeedLow                = 0x40;
const uint8_t kSevilleSpeedMedium             = 0x41;
const uint8_t kSevilleSpeedHigh               = 0x42;

// Digit 10, Byte 9 - "Wind" mode
const uint8_t kSevilleWindNormal              = 0x50;
const uint8_t kSevilleWindNatural             = 0x51;
const uint8_t kSevilleWindSleeping            = 0x52;

class IRSevilleFan {
 public:
  explicit IRSevilleFan(uint16_t pin);

  void send(const uint16_t repeat = kSevilleDefaultRepeat);

  void begin();

  void setPower(bool state);
  bool getPower();

  void setTimer(uint8_t timer);
  uint8_t getTimer();

  void setOscillation(bool osc);
  bool getOscillation();

  void setSpeed(uint8_t speed);
  uint8_t getSpeed();

  void setWind(uint8_t wind);
  uint8_t getWind();

  uint8_t* getRaw();

 private:
  uint8_t remote_state[kSevilleStateLength];
  void stateReset();
  void checksum();
  IRsend _irsend;
  void sendSeville(uint64_t data, uint16_t nbits = kSevilleBits,
                   uint16_t repeat = kSevilleDefaultRepeat);
  void sendSeville(unsigned char data[], uint16_t nbytes = kSevilleStateLength,
                   uint16_t repeat = kSevilleDefaultRepeat);
};

#endif  // SEVILLE_H_
