/*  Based on the 'Send Temperature with LoRaWAN' example
 *  from http://support.sodaq.com/sodaq-one/lorawan/
 *  Modifications by Gabriel Notman
 */

#include "Sodaq_RN2483.h"
#include "Utils.h"
#include "Shared.h"
#include "BLE_Configurator.h"

void setup()
{
  while ((!debugSerial) && (millis() < 10000)){
    // Wait 10 seconds for debugSerial to open
  }

  debugSerial.println("Starting...");

  // Start streams
  debugSerial.begin(57600);
  loraSerial.begin(LoRaBee.getDefaultBaudRate());

  LoRaBee.setDiag(debugSerial); // to use debug remove //DEBUG inside library
  LoRaBee.init(loraSerial, LORA_RESET);
  
  //Print Original Keys
  printKeys("Original Keys:");

  // Get configuration over BLE
  setupBLE();
  if (!configOverBLE()) {
    debugSerial.println("BLE Configuration. Failed");
    while(1);
  }

  debugSerial.println("BLE Configuration. Success");
    
  //Print configured Keys
  printKeys("Configured Keys:");
 
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

  // Print the payload
  debugSerial.print("Packet Payload: ");
  debugSerial.print(reading + " = ");
  printHex((uint8_t*)reading.c_str(), reading.length());
  debugSerial.println();
  

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

void printKeys(String header)
{
  // Print the header
  debugSerial.println("--------------------------------");
  debugSerial.println(header);
  
  // Print the App EUI
  debugSerial.print("LoRa AppvEUI: ");
  printHex(AppEUI, sizeof(AppEUI));
  debugSerial.println();
  
  // Print the Dev EUI
  debugSerial.print("LoRa DevEUI: ");
  printHex(DevEUI, sizeof(DevEUI));
  debugSerial.println();

  // Print the App Key
  debugSerial.print("LoRa AppKey: ");
  printHex(AppKey, sizeof(AppKey));
  debugSerial.println();
  debugSerial.println("--------------------------------");
}

void printHex(uint8_t* buff, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
      debugSerial.print((char)NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(buff[i])));
      debugSerial.print((char)NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(buff[i])));
    }
}

