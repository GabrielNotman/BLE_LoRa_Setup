#include "BLE_Configurator.h"
#include "RN487x_BLE.h"
#include "Shared.h"
#include "Utils.h"

#define DEVICE_PREFIX "SODAQ_EXPLORER"
#define STATUS_PREFIX "[STATUS MSG]: "
#define DATA_PREFIX "[DATA_MSG]: "
#define RESP_PREFIX "[RESPONSE]: "

#define CMD_STATUS "AT"
#define CMD_KEYS "CMD1="

#define NEW_LINE "\r\n"
#define CMD_SEP '='
#define KEY_SEP '|'
#define STATUS_SEP '%'

#define SUCCESS_RESP "OK"
#define FAIL_RESP "ERROR"

#define CONFIG_TIMEOUT 60000
#define LINE_TIMEOUT 1000

#define BUFF_LEN 128

uint8_t buff[BUFF_LEN];
uint8_t buffLen = 0;
bool configured = false;

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

void setupBLE()
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
    debugSerial.println("BLE Init. Success!") ;
  }
  else
  {
    setRgbColor(255, 255, 0) ;
    debugSerial.println("BLE Init. Failed!") ;
    while(1) ;
  }

  rn487xBle.enterCommandMode();
  rn487xBle.setSerializedName(DEVICE_PREFIX) ;
  rn487xBle.setDefaultServices(DEVICE_INFO_SERVICE | UART_TRANSP_SERVICE);
  rn487xBle.reboot();
}

void readLn()
{
  buffLen = 0;
  bool statusDetected = false;
  bool seenCR = false;
  
  //Timeout function
  int32_t timeOut = millis() + LINE_TIMEOUT;
  while ((millis() < timeOut) && (!seenCR)) {
    if (bleSerial.available()) {
      char c = bleSerial.read();

      //Write character to debug or buffer depending on status state
      if (statusDetected) {
        debugSerial.print(c);
      }
      //Skip if about to enter status state
      else if ((buffLen < (BUFF_LEN - 1)) && (c != STATUS_SEP)) {
        //Check for CR        
        seenCR = c == '\r';
        if (seenCR) {
          delay(10);
          //Check for LF
          if (bleSerial.peek() == '\n') {
            bleSerial.read();
          }
        }
        else {
          buff[buffLen] = c;
          buffLen++;
        }
      }

      //If status terminator is detected
      if (c == STATUS_SEP) {
        //Invert status message state
        statusDetected = !statusDetected;

        //Handle each transition separately
        if (statusDetected) {
          debugSerial.print(STATUS_PREFIX);
          debugSerial.print(c);
        }
        else {
          debugSerial.println();
        }
      }
      
      //Reset Timeout if a character is seen
      int32_t timeOut = millis() + LINE_TIMEOUT;
    }
  }
  
  //Add terminating 0
  buff[buffLen] = 0;
}

bool processKeyCMD(String command)
{
  //The format is (AppEUI|DevEUI|AppKey).
  //0090D31000000002|0090D32000000004|4613F695AF37D2C4691786C59A78EC94

  bool fail = false;
  uint8_t start = 0;
  uint8_t end = 0;
  
  //AppEUI
  if (!fail) {
    start = command.indexOf(CMD_SEP) + 1;
    end = command.indexOf(KEY_SEP);

    //debugSerial.println(String(start) + " " + String(end) + " " + String(end - start) + " " + String(sizeof(AppEUI)*2));
    
    if ((end - start) == (sizeof(AppEUI)*2)) {
      for (uint8_t i = 0; i < sizeof(AppEUI); i++) {
        uint8_t index = start+i*2;
        AppEUI[i] = HEX_PAIR_TO_BYTE(command[index],command[index + 1]);
      }
    }
    else {
      fail = true;
    }    
  }

  //DevEUI
  if (!fail) {
    start = end + 1;
    end = command.indexOf(KEY_SEP, start);

    //debugSerial.println(String(start) + " " + String(end) + " " + String(end - start) + " " + String(sizeof(DevEUI)*2));
    
    if ((end - start) == (sizeof(DevEUI)*2)) {
      for (uint8_t i = 0; i < sizeof(DevEUI); i++) {
        uint8_t index = start+i*2;
        DevEUI[i] = HEX_PAIR_TO_BYTE(command[index],command[index + 1]);
      }
    }
    else {
      fail = true;
    }
  }

  //AppKey
  if (!fail) {
    start = end + 1;
    end = command.length();

    //debugSerial.println(String(start) + " " + String(end) + " " + String(end - start) + " " + String(sizeof(AppKey)*2));
    
    if ((end - start) == (sizeof(AppKey)*2)) {
      for (uint8_t i = 0; i < sizeof(AppKey); i++) {
        uint8_t index = start+i*2;
        AppKey[i] = HEX_PAIR_TO_BYTE(command[index],command[index + 1]);
      }

      //If we get here, all the keys have been configured
      configured = true;
    }
    else {
      fail = true;
    }
  }

  return !fail;
}

void processLn()
{
  if (buffLen > 0) {
    String command((char*)buff);
    debugSerial.print(DATA_PREFIX);
    debugSerial.println(command);

    String response = FAIL_RESP;

    //Messy hack for now, just specific command comparisons
    
    //AT
    if (command.equals(CMD_STATUS)) {
      response = SUCCESS_RESP;
    }

    //CMD1
    if (command.startsWith(CMD_KEYS)) {
      
      if (processKeyCMD(command)) {
        response = SUCCESS_RESP;
      }
    }

    bleSerial.print(response);
    bleSerial.print(NEW_LINE);

    debugSerial.print(RESP_PREFIX);
    debugSerial.println(response);
  }
}

bool configOverBLE()
{
  //Clear the reboot character 0x0A
  bleSerial.read();
  
  int32_t timeOut = millis() + CONFIG_TIMEOUT;
  
  while (!configured && (millis() < timeOut)) {
    readLn();
    processLn();
  }

  //Shutdown BLE module
  rn487xBle.hwReset();

  if (configured) {
    setRgbColor(0, 255, 0) ;  
  }
  else {
    setRgbColor(255, 0, 0) ;
  }
  
  return configured; 
}

