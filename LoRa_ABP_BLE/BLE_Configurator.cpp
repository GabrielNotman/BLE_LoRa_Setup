#include "BLE_Configurator.h"
#include "RN487x_BLE.h"

#define DEVICE_PREFIX "SODAQ_EXPLORER"
#define NEW_LINE "\r\n"
#define CMD_SEP "="
#define STATUS_SEP "%"

void initLed()
{
  pinMode(LED_RED, OUTPUT) ;
  pinMode(LED_GREEN, OUTPUT) ;
  pinMode(LED_BLUE, OUTPUT) ;  
}

void setRgbColor(uint8_t red, uint8_t green, uint8_t blue)
{
  red = 255 - red ;
  green = 255 - green ;
  blue = 255 - blue ;

  analogWrite(LED_RED, red) ;
  analogWrite(LED_GREEN, green) ;
  analogWrite(LED_BLUE, blue) ;
}

void setupBLE(Uart& bleSerial, Stream& debugSerial)
{
  initLed();
  setRgbColor(255, 255, 255);
  rn487xBle.setDiag(debugSerial) ;
  rn487xBle.hwInit();
  bleSerial.begin(rn487xBle.getDefaultBaudRate()) ;
  rn487xBle.initBleStream(&bleSerial) ;
  if (rn487xBle.swInit())
  {
    setRgbColor(0, 0, 255) ;
    debugSerial.println("BLE Init. procedure done!") ;
  }
  else
  {
    setRgbColor(255, 255, 0) ;
    debugSerial.println("BLE Init. procedure failed!") ;
    while(1) ;
  }

  rn487xBle.enterCommandMode();
  rn487xBle.setSerializedName(DEVICE_PREFIX) ;
  rn487xBle.setDefaultServices(DEVICE_INFO_SERVICE | UART_TRANSP_SERVICE);
  rn487xBle.reboot();

  while(1) {
    while (bleSerial.available()) {
      debugSerial.write((char)bleSerial.read());
    }

    while (debugSerial.available()) {
      bleSerial.write((char)debugSerial.read());
    }
  }
}

bool configOverBLE()
{
  return false; 
}

