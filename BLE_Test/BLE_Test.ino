// Scanner.ino
// Author: M16946
// Date: 2017/01/13

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include <Arduino.h>
#include "RN487x_BLE.h"

#define debugSerial SerialUSB
#define bleSerial Serial1

#define SERIAL_TIMEOUT  10000

void initLed()
{
  #if defined(ARDUINO_SODAQ_EXPLORER)
    pinMode(LED_BUILTIN, OUTPUT) ;
  #endif
  pinMode(LED_RED, OUTPUT) ;
  pinMode(LED_GREEN, OUTPUT) ;
  pinMode(LED_BLUE, OUTPUT) ;  
  pinMode(BUTTON, INPUT_PULLUP) ;
}

void turnBlueLedOn()
{
  #if defined(ARDUINO_SODAQ_EXPLORER)
    digitalWrite(LED_BUILTIN, HIGH) ;
  #endif
}

void turnBlueLedOff()
{
  #if defined(ARDUINO_SODAQ_EXPLORER)
    digitalWrite(LED_BUILTIN, LOW) ;
  #endif
}

#define COMMON_ANODE  // LED driving
void setRgbColor(uint8_t red, uint8_t green, uint8_t blue)
{
#ifdef COMMON_ANODE
  red = 255 - red ;
  green = 255 - green ;
  blue = 255 - blue ;
#endif

  analogWrite(LED_RED, red) ;
  analogWrite(LED_GREEN, green) ;
  analogWrite(LED_BLUE, blue) ;
}

void setup()
{
  while ((!debugSerial) && (millis() < SERIAL_TIMEOUT)) ;
  
  debugSerial.begin(115200) ;

  initLed() ;

  rn487xBle.setDiag(debugSerial) ;
  rn487xBle.hwInit() ;
  bleSerial.begin(rn487xBle.getDefaultBaudRate()) ;
  rn487xBle.initBleStream(&bleSerial) ;
  if (rn487xBle.swInit())
  {
    setRgbColor(0, 255, 0) ;
    debugSerial.println("BLE Init. procedure done!") ;
  }
  else
  {
    setRgbColor(255, 0, 0) ;
    debugSerial.println("BLE Init. procedure failed!") ;
    while(1) ;
  }

  rn487xBle.enterCommandMode();
  rn487xBle.retrieveBtAddress();
  rn487xBle.setSerializedName("Explorer") ;
  rn487xBle.setDefaultServices(DEVICE_INFO_SERVICE | UART_TRANSP_SERVICE);
  rn487xBle.reboot();

  debugSerial.print("Your Device's Name: ");
  debugSerial.print(rn487xBle.getDeviceName());
  debugSerial.print("-");
  debugSerial.println(&rn487xBle.getBtAddress()[8]);
  
  //Standard Nordic, does not actually open a stream
  //rn487xBle.setServiceUUID("6E400001B5A3F393E0A9E50E24DCCA9E");
  //rn487xBle.setCharactUUID("6E400002B5A3F393E0A9E50E24DCCA9E", WRITE_PROPERTY, 20) ;
  //rn487xBle.setCharactUUID("6E400003B5A3F393E0A9E50E24DCCA9E", READ_PROPERTY | NOTIFY_PROPERTY, 20) ;

  //Default not actually required to be set
  //rn487xBle.enterCommandMode();
  //rn487xBle.setServiceUUID("49535343FE7D4AE58FA99FAFD205E455");
  //rn487xBle.setCharactUUID("495353431E4D4BD9BA6123C647249616", WRITE_PROPERTY, 20);
  //rn487xBle.setCharactUUID("49535343884143F4A8D4ECBE34729BB3", READ_PROPERTY | NOTIFY_PROPERTY, 20);
  //rn487xBle.reboot();
}

void loop()
{
  // Display the result of the scanning
  if (bleSerial.available())
  {
    debugSerial.write((char)bleSerial.read());
  }
}
