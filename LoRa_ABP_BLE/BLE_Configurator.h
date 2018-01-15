#ifndef _BLE_CONFIGURATOR_H
#define _BLE_CONFIGURATOR_H

#include <stdint.h>

static uint8_t DevEUI[8]
{
  0x00, 0x12, 0xBF, 0x29, 0x4C, 0x67, 0xF7, 0x77
  //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static uint8_t AppEUI[8] =
{
  0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0x74, 0xF1
  //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t AppKey[16] =
{
  0xDB, 0x5A, 0xE4, 0x88, 0x87, 0x4C, 0xA8, 0x61, 0x23, 0x1C, 0xB4, 0x81, 0x37, 0x7B, 0x9D, 0x7C
  //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setupBLE();
bool configOverBLE();

#endif
