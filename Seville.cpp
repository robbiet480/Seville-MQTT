// Copyright 2019 Robbie Trencheny (@robbie)
//
// Code to control Seville Classics Ultra Slimline Fan.

#include "Seville.h"
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRutils.h"

IRSevilleFan::IRSevilleFan(uint16_t pin) : _irsend(pin) { stateReset(); }

void IRSevilleFan::begin() { _irsend.begin(); }

void IRSevilleFan::send(const uint16_t repeat) {
  checksum();
  sendSeville(remote_state);
}

void IRSevilleFan::checksum() {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < kSevilleStateLength - 1; i++) checksum ^= remote_state[i];
  remote_state[7] = checksum;
}

void IRSevilleFan::stateReset() {
  for (uint8_t i = 0; i < kSevilleStateLength; i++) remote_state[i] = 0x00;

  remote_state[5] = 0x00;
  remote_state[6] = 0x60;
  remote_state[7] = 0x70;

  setPower(false);
  setSpeed(kSevilleSpeedEco);
  setOscillation(false);
  setTimer(kSevilleTimerNone);
  setWind(kSevilleWindNormal);
}

uint8_t* IRSevilleFan::getRaw() {
  checksum();
  return remote_state;
}

void IRSevilleFan::setPower(bool state) {
  if (state)
    remote_state[0] = kSevillePowerOn;
  else
    remote_state[0] = kSevillePowerOff;
}

bool IRSevilleFan::getPower() {
  return remote_state[0] == kSevillePowerOn;
}

void IRSevilleFan::setTimer(uint8_t timer) {
  // if (timer > kSevilleMaxTimer) timer = kSevilleMaxTimer;

  if (timer) {
    remote_state[1] = timer;
  } else {
    remote_state[1] = kSevilleTimerNone;
  }
}

uint8_t IRSevilleFan::getTimer() { return remote_state[1]; }

void IRSevilleFan::setOscillation(bool osc) {
  if (osc)
    remote_state[2] = kSevilleOscillationOn;
  else
    remote_state[2] = kSevilleOscillationOff;
}

bool IRSevilleFan::getOscillation() {
  return remote_state[2] == kSevilleOscillationOn;
}

void IRSevilleFan::setSpeed(uint8_t speed) {
  if (speed)
    remote_state[3] = speed;
  else
    remote_state[3] = kSevilleSpeedEco;
}

uint8_t IRSevilleFan::getSpeed() { return remote_state[3]; }

void IRSevilleFan::setWind(uint8_t wind) {
  if (wind)
    remote_state[4] = wind;
  else
    remote_state[4] = kSevilleWindNormal;
}

uint8_t IRSevilleFan::getWind() { return remote_state[4]; }

// Send a Seville Classics message.
//
// Args:
//   data: The raw message to be sent.
//   nbytes: Nr. of bytes in the array. (Default is kSevilleStateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
void IRSevilleFan::sendSeville(unsigned char data[], uint16_t nbytes, uint16_t repeat) {
  if (nbytes != kSevilleStateLength)
    return;  // Wrong nr. of bits to send a proper message.
  // Set IR carrier frequency

  _irsend.sendGeneric(kSevilleHdrMark, kSevilleHdrSpace,
                      kSevilleBitMark, kSevilleOneSpace,
                      kSevilleBitMark, kSevilleZeroSpace,
                      kSevilleBitMark, kSevilleMsgSpace,
                      data, nbytes,
                      38, true,
                      repeat, 50);
}

void IRSevilleFan::sendSeville(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits != kSevilleBits)
    return;  // Wrong nr. of bits to send a proper message.

  // Set IR carrier frequency
  _irsend.enableIROut(38, 50);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    _irsend.mark(kSevilleHdrMark);
    _irsend.space(kSevilleHdrSpace);

    _irsend.sendData(kSevilleBitMark, kSevilleOneSpace, kSevilleBitMark, kSevilleZeroSpace, data, nbits, true);

    _irsend.mark(kSevilleBitMark);
    _irsend.space(kSevilleMsgSpace);
  }
}
