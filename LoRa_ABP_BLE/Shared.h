#ifndef _SHARED_H
#define _SHARED_H

#include <Arduino.h>
#include <stdint.h>

#define debugSerial SerialUSB
#define loraSerial Serial2
#define bleSerial Serial1

extern uint8_t DevEUI[8];
extern uint8_t AppEUI[8];
extern uint8_t AppKey[16];

#endif
