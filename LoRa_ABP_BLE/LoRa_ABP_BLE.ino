/*  Based on the 'Send Temperature with LoRaWAN' example
 *  from http://support.sodaq.com/sodaq-one/lorawan/
 *  Modifications by Gabriel Notman
 */

#include "Sodaq_RN2483.h"

#define debugSerial SerialUSB
#define loraSerial Serial2

#define NIBBLE_TO_HEX_CHAR(i) ((i <= 9) ? ('0' + i) : ('A' - 10 + i))
#define HIGH_NIBBLE(i) ((i >> 4) & 0x0F)
#define LOW_NIBBLE(i) (i & 0x0F)

// OTAA
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

void setup()
{
  delay(1000);
  
  while ((!debugSerial) && (millis() < 10000)){
    // Wait 10 seconds for debugSerial to open
  }

  debugSerial.println("Start");

  // Start streams
  debugSerial.begin(57600);
  loraSerial.begin(LoRaBee.getDefaultBaudRate());

  LoRaBee.setDiag(debugSerial); // to use debug remove //DEBUG inside library
  LoRaBee.init(loraSerial, LORA_RESET);

  //Use the Hardware EUI
  //getHWEUI();

  // Print the Hardware EUI
  debugSerial.print("LoRa HWEUI: ");
  for (uint8_t i = 0; i < sizeof(DevEUI); i++) {
    debugSerial.print((char)NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(DevEUI[i])));
    debugSerial.print((char)NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(DevEUI[i])));
  }
  debugSerial.println();  
  
  setupLoRa();
}

void setupLoRa(){
  if (LoRaBee.initOTA(loraSerial, DevEUI, AppEUI, AppKey, true))
  {
    debugSerial.println("Communication to LoRaBEE successful.");
  }
  else
  {
    debugSerial.println("Communication to LoRaBEE failed!");
  }
  LoRaBee.setSpreadingFactor(9);
}

void loop()
{
  String reading = getTemperature();
  debugSerial.println(reading);

  switch (LoRaBee.send(1, (uint8_t*)reading.c_str(), reading.length())) {
    case NoError:
      debugSerial.println("Successful transmission.");
      break;
    case NoResponse:
      debugSerial.println("There was no response from the device.");
      break;
    case Timeout:
      debugSerial.println("Connection timed-out. Check your serial connection to the device! Sleeping for 20sec.");
      delay(20000);
      break;
    case PayloadSizeError:
      debugSerial.println("The size of the payload is greater than allowed. Transmission failed!");
      break;
    case InternalError:
      debugSerial.println("Oh No! This shouldn't happen. Something is really wrong! The program will reset the RN module.");
      setupLoRa();
      break;
    case Busy:
      debugSerial.println("The device is busy. Sleeping for 10 extra seconds.");
      delay(10000);
      break;
    case NetworkFatalError:
      debugSerial.println("There is a non-recoverable error with the network connection. The program will reset the RN module.");
      setupLoRa();
      break;
    case NotConnected:
      debugSerial.println("The device is not connected to the network. The program will reset the RN module.");
      setupLoRa();
      break;
    case NoAcknowledgment:
      debugSerial.println("There was no acknowledgment sent back!");
      break;
    default:
      break;
    }
    // Delay between readings
    // 60 000 = 1 minute
    delay(10000); 
}

String getTemperature()
{
  //10mV per C, 0C is 500mV
  float mVolts = (float)analogRead(TEMP_SENSOR) * 3300.0 / 1023.0;
  float temp = (mVolts - 500.0) / 10.0;
  
  return String(temp);
}

/**
* Gets and stores the LoRa module's HWEUI/
* Unused
*/
static void getHWEUI()
{
  uint8_t len = LoRaBee.getHWEUI(DevEUI, sizeof(DevEUI));
}
