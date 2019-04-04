#include "Display.h"
#include "Miscellaneous.h"
#include "SeeedGrayOLED.h"
#include "Clock.h"
#include "Watering.h"
  
/*
=================================================================
|| Run the function that corresponds to selected display mode. ||
================================================================= */
void Display::printToScreen(unsigned short moistureValue1, unsigned short moistureValue2, unsigned short moistureValue3, unsigned short moistureValue4, unsigned short moistureMeanValue, unsigned short tempValue, unsigned short humidityValue, uint16_t lightValue, uint16_t uvValue) {
  SeeedGrayOled.setVerticalMode();            //Display must be set vertical addressing mode to make any text printed to display, readable.
  switch(displayState) {
    case STARTUP_IMAGE:
      viewStartupImage();                     //Initialize the OLED Display and print startup images to display.
      break;
    case SET_CLOCK:
      stringToDisplay(0, 7, "SET CLOCK");     //Print current display state to upper right corner of display.
      viewSetClock();                         //Display "set time" screen.
      Clock::getInstance().setTime();         //Set current time.
      break;
    case READOUT_VALUES:
      stringToDisplay(0, 2, "READOUT VALUES");//Print current display state to upper right corner of display.
      //viewReadoutValues();                  //Print read out values from the greenhouse to display.
      break;
    case SERVICE_MODE:
      stringToDisplay(0, 4, "SERVICE MODE");  //Print current display state to upper right corner of display.
      //viewServiceMode();                    //Service mode screen is printed to display.
      break;
    case FLOW_FAULT:
      stringToDisplay(0, 6, "FLOW FAULT");    //Print current display state to upper right corner of display.
      //waterFlowFault = resolveFlowFault();  //Water flow fault display mode is printed to display. It contains fault code instruction and possibility to reset fault code.  
      break;
  }
}

/*
======================================================================================
|| Toggle set modes and screen display modes when clockModeButton is being pressed. ||
====================================================================================== */
 void Display::toggleDisplayMode() {
  if((millis() - pressTimePrev) >= DEBOUNCE_TIME_PERIOD) {  //Debouncing button press to avoid multiple interrupts, display toggles.

    //Check if water flow fault code is active. If it is set 'true', FLOW_FAULT display will be activated next time mode-button is being pressed to let user resolve the fault code.
    if(waterFlowFault == true) {
      displayState = FLOW_FAULT;                //Set next display mode to be printed to display. 
    }
    switch(displayState) {
      case SET_CLOCK:
        switch(clockInputState) {
          case HOUR2:
            clockInputState = HOUR1;              //Hour pointer2 has been set. Continue by setting hour pointer1.
            Config::State().clockInputState_x = 1;                //Temp variable because of typedef could not be global variable.
            Serial.println("hour1InputMode");
            break;
          case HOUR1:
            clockInputState = MINUTE2;            //Hour pointer1 has been set. Continue by setting minute pointer2.
            Config::State().clockInputState_x = 2;                 //Temp variable because of typedef could not be global variable.
            Serial.println("minute2InputMode");
            break;
          case MINUTE2:
            clockInputState = MINUTE1;            //Minute pointer2 has been set. Continue by setting minute pointer1.
            Config::State().clockInputState_x = 3;                 //Temp variable because of typedef could not be global variable.
            Serial.println("minute1InputMode");
            break;
          case MINUTE1:
            clockInputState = COMPLETED;          //Clock setting completed.
            Config::State().clockStartEnabled = true;             //Start clock. Clock starts ticking.
            Serial.println("clockSettingCompleted");
            break;
          case COMPLETED:
            displayState = READOUT_VALUES;        //Set next display mode to be printed to display.
            alarmMessageEnabled = true;           //Enable any alarm message to be printed to display.
            greenhouseProgramStart = true;        //Start greenhouse program.
            Serial.println("READOUT_VALUES");
            break;           
        }
        break;
      case READOUT_VALUES:
        displayState = SERVICE_MODE;              //Set next display mode to be printed to display.
        alarmMessageEnabled = false;              //Disable any alarm message from being printed to display.
        Serial.println("SERVICE_MODE");
        break;
      case SERVICE_MODE:
        displayState = READOUT_VALUES;            //Set next display mode to be printed to display.
        alarmMessageEnabled = false;              //Disable any alarm message from being printed to display.
        Serial.println("READOUT_VALUES");     
        break;
      case FLOW_FAULT:
        greenhouseProgramStart = false;           //Stop greenhouse program.
        alarmMessageEnabled = false;              //Disable any alarm message from being printed to display.
        Serial.println("FLOW_FAULT"); 

        //In flow fault display mode user will have the chance to clear the fault code. Check if water flow fault code has been cleared by user or not.
        if(waterFlowFault == false) {               //Fault code has been cleared by user. Continue to run greenhouse program.               
          displayState = SERVICE_MODE;              //Since using interrupt for toggeling display mode, next display mode to be printed to display is READOUT_VALUES even though SERVICE_MODE display is set active mode here.
          greenhouseProgramStart = true;            //Continue greenhouse program.
          alarmMessageEnabled = true;               //Enable any alarm message from being printed to display.
          Serial.println("Resume program");  
        }
        else if(waterFlowFault == true) {           //If flow fault code has not been cleared, reboot greenhouse program.
          waterFlowFault = false;                   //Clear water flow fault code.
          displayState = STARTUP_IMAGE;             //Set next display mode to be printed to display.  
          //Watering::getInstance().stopPump();       //Stop water pump.
          //Clock::getInstance().resetTime();         //Reset clock time.   //MOVE TO OTHER FUNCTION INSTEAD!
          Serial.println("Reboot program");             
        }
        break;
    }
    pressTimePrev = millis();
  }
}

/*
===================================
|| Print custom text to display. ||
=================================== */
void Display::stringToDisplay(unsigned char x, unsigned char y, char* text) {
  y *= 8;                                         //To align symbol with rest printed text. Each symbol requires 8px in width.
  SeeedGrayOled.setTextXY(x, y);                  //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString(text);                  //Print text to display.
}

/*
=======================================
|| Print number variable to display. ||
======================================= */
void Display::numberToDisplay(unsigned char x, unsigned char y, unsigned short variable) {
  y *= 8;                                         //To align symbol with rest printed text. Each symbol requires 8px in width.
  SeeedGrayOled.setTextXY(x, y);                  //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(variable);              //Print value to display.
}

/*
======================================================
|| Clear any character/s (print blanks) at display. ||
====================================================== */
void Display::blankToDisplay(unsigned char x, unsigned char y, int numOfBlanks) {
  y *= 8;                                         //To align symbol with rest printed text. Each symbol requires 8px in width.
  for(int i=0; i<numOfBlanks; i++) {              //Print blank space to display. Each loop one blank space is printed.
    SeeedGrayOled.setTextXY(x, y);                //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
    SeeedGrayOled.putString(" ");                 //Blank symbol.
    y++;                                          //Increase column cordinate to print next blank space in the same row.
  }
}

/*
=========================================================================================
|| Alternate print and clear intiger value (flash) to display with a frequency of 2Hz. ||
========================================================================================= */
void Display::flashNumberDisplay(unsigned short x, unsigned short y, unsigned short variable) {
  unsigned short numDigits;               //Store number of digits in variable value;
  y *= 8;                                     //To align symbol with rest printed text. Each symbol requires 8px in width.
  if(variable == 0) {                         //If variable equal to 0 it contains 1 digit.
    numDigits = 1;
  }
  while(variable != 0) {
    numDigits++;
    variable /= 10;                           //Result of division between values with type 'int' only gives an integer. If variable less than 10 result of division is 0.
  }

  //Toggle between printing variable value and printing blank space to make number appear to flash on display.
  switch(Config::State().flash) {
    case true:
      for(int i=0; i<numDigits; i++) {          //Print blank space to display. Each loop one blank space is printed.
        SeeedGrayOled.setTextXY(x, y);          //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
        SeeedGrayOled.putString(" ");           //Blank symbol.
        y += 8;                                 //Increase column cordinate to print next blank space in the same row.
      }
      break;
    case false:
      SeeedGrayOled.setTextXY(x, y);            //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
      SeeedGrayOled.putNumber(variable);        //Print variable value.
      break;
  }
}

/*
=========================================================================================
|| Alternate print and clear intiger value (flash) to display with a frequency of 2Hz. ||
========================================================================================= */
void Display::flashCharacterDisplay(unsigned short x, unsigned short y, char symbol) {
  y *= 8;                                     //To align symbol with rest printed text. Each symbol requires 8px in width.

  //Toggle between printing symbol and printing blank space to make symbol appear to flash on display.
  switch(Config::State().flash) {
    case true:
      SeeedGrayOled.setTextXY(x, y);          //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
      SeeedGrayOled.putString(" ");           //Blank symbol.
      break;
    case false:
      SeeedGrayOled.setTextXY(x, y);            //Set cordinates to where text will be printed. X = row (0-7), Y = column (0-127).
      SeeedGrayOled.putChar(symbol);            //Print variable value.
      break;
  }
}

/*
=========================================================================
|| Print current clock values to display to let user set current time. ||
========================================================================= */
void Display::viewSetClock() {
  
  stringToDisplay(2, 0, "Set current time");
  stringToDisplay(3, 0, "Use the buttons:");
  stringToDisplay(5, 0, "SET = inc. p.val");
  stringToDisplay(6, 0, "MODE = h or min");
 
  stringToDisplay(9, 4, "HH MM SS");
  //Flash separator between hour-minute and minute-second cursors when clock is ticking.
  if(Config::State().clockStartEnabled == true) {
    flashCharacterDisplay(10, 6, ':');
    flashCharacterDisplay(10, 9, ':');
  }
  else {
    stringToDisplay(10, 6, ":");
    stringToDisplay(10, 9, ":");
  }

  numberToDisplay(10, 4, Config::State().hourCursor2);
  numberToDisplay(10, 5, Config::State().hourCursor1);
  numberToDisplay(10, 7, Config::State().minuteCursor2);
  numberToDisplay(10, 8, Config::State().minuteCursor1);
  numberToDisplay(10, 10, Config::State().secondCursor2);
  numberToDisplay(10, 11, Config::State().secondCursor1);
  
  //Flash (show/clear) individual clock cursors to display which clock parameter that is currently being set.
  switch(clockInputState) {
    case HOUR2:
      flashNumberDisplay(10, 4, Config::State().hourCursor2);    //Toggle show/clear clock cursor with a frequency of 2 Hz.
      break;
    case HOUR1:
      flashNumberDisplay(10, 5, Config::State().hourCursor1);    //Toggle show/clear clock cursor with a frequency of 2 Hz.
      break;
    case MINUTE2:
      flashNumberDisplay(10, 7, Config::State().minuteCursor2);  //Toggle show/clear clock cursor with a frequency of 2 Hz.
      break;
    case MINUTE1:
      flashNumberDisplay(10, 8, Config::State().hourCursor1);    //Toggle show/clear clock cursor with a frequency of 2 Hz.
      break;
  }
}

  /*
  -----------------------------------------------------------------------------
  |Bitmap image (stored as an array) to be printed on OLED display at startup.|
  ----------------------------------------------------------------------------- */
  const unsigned char greenhouse[] PROGMEM= {
  //Startup image.
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x07, 0xC0,
0x00, 0x03, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x04, 0xA0,
0x00, 0x06, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x04, 0xA0,
0x00, 0x0C, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x40, 0x00, 0x03, 0x00,
0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0xFC, 0x7C, 0x00, 0x04, 0x00,
0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80,
0x00, 0x04, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC0, 0xF0, 0x38, 0x00, 0x07, 0x80,
0x00, 0x06, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x21, 0x00, 0x44, 0x00, 0x00, 0x80,
0x00, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x44, 0x00, 0x00, 0x80,
0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x38, 0x00, 0x07, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x07, 0x81, 0x00, 0x00, 0x00, 0x03, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x0A, 0x81, 0x00, 0x08, 0x00, 0x04, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x54, 0x00, 0x04, 0x80,
0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x01, 0x01, 0xF0, 0x24, 0x00, 0x1F, 0x80,
0x00, 0x0C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80,
0x00, 0x0C, 0x31, 0x80, 0x00, 0x00, 0x00, 0x12, 0x00, 0x7F, 0x81, 0xF0, 0x3C, 0x00, 0x04, 0x00,
0x00, 0x0C, 0x61, 0x80, 0x00, 0x00, 0x00, 0x0C, 0x08, 0x00, 0x00, 0x10, 0x40, 0x00, 0x04, 0x00,
0x00, 0x0C, 0x61, 0x80, 0x00, 0x00, 0x00, 0x04, 0x0F, 0x07, 0x00, 0x10, 0x40, 0x00, 0x07, 0x80,
0x00, 0x07, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x1A, 0x06, 0x08, 0x81, 0xE0, 0x7C, 0x00, 0x03, 0x00,
0x00, 0x03, 0x81, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x09, 0x08, 0x80, 0x00, 0x00, 0x00, 0x05, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x08, 0x83, 0x00, 0x34, 0x00, 0x05, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x09, 0x0F, 0xE4, 0x80, 0x54, 0x00, 0x03, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x06, 0x00, 0x04, 0x80, 0x54, 0x00, 0x07, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x0A, 0x09, 0x07, 0xF0, 0x38, 0x00, 0x04, 0x00,
0x00, 0x0F, 0xFF, 0x8F, 0xF8, 0x00, 0x00, 0x00, 0x0D, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x1F, 0x80,
0x00, 0x0F, 0xFF, 0x80, 0x30, 0x00, 0x00, 0x0A, 0x05, 0x04, 0x80, 0x00, 0x18, 0x00, 0x00, 0x00,
0x00, 0x00, 0x02, 0x00, 0xC0, 0x00, 0x00, 0x1A, 0x07, 0x00, 0x00, 0x01, 0x24, 0x00, 0x04, 0x80,
0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x1A, 0x08, 0x2F, 0x81, 0x00, 0xC4, 0x00, 0x0F, 0x00,
0x00, 0x00, 0x01, 0x86, 0x00, 0x00, 0x00, 0x0C, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00,
0x00, 0x00, 0x01, 0x8F, 0xF8, 0x00, 0x00, 0x0C, 0x0F, 0x0E, 0x01, 0xF0, 0x00, 0x00, 0x04, 0x80,
0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x4A, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x04, 0x80,
0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x52, 0x05, 0x20, 0x80, 0xD0, 0x70, 0x00, 0x03, 0x00,
0x00, 0x0F, 0xFF, 0x08, 0x88, 0x00, 0x00, 0x22, 0x0D, 0x20, 0x81, 0x50, 0x0C, 0x00, 0x03, 0x80,
0x00, 0x0F, 0xFC, 0x08, 0x88, 0x00, 0x00, 0x00, 0x06, 0x20, 0x81, 0x50, 0x32, 0x00, 0x0A, 0x80,
0x00, 0x00, 0x00, 0x08, 0x88, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0xE0, 0x40, 0x00, 0x08, 0x80,
0x00, 0x00, 0x00, 0x0F, 0xF8, 0x00, 0x00, 0x12, 0x26, 0x00, 0x00, 0x00, 0x44, 0x00, 0x07, 0x00,
0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x29, 0x00, 0x01, 0x10, 0xFC, 0x00, 0x00, 0x00,
0x00, 0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x03, 0xF0, 0x40, 0x00, 0x00, 0x00,
0x00, 0x1F, 0xFE, 0x08, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x00, 0xC0,
0x00, 0x1F, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x10, 0x00, 0x11, 0x00, 0xF1, 0x7C, 0x00, 0x00, 0x00,
0x00, 0x0F, 0xFF, 0x0F, 0xF8, 0x00, 0x00, 0x7E, 0x00, 0x20, 0x81, 0x50, 0x00, 0x00, 0x0F, 0x00,
0x00, 0x0F, 0xDF, 0x88, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x20, 0x81, 0x51, 0xFC, 0x00, 0x09, 0x80,
0x00, 0x07, 0xEF, 0x88, 0x00, 0x00, 0x00, 0x1F, 0x04, 0x20, 0x80, 0x30, 0x44, 0x00, 0x0E, 0x80,
0x00, 0x07, 0xF7, 0x88, 0x08, 0x00, 0x00, 0x12, 0x8D, 0x3F, 0x80, 0x00, 0x44, 0x10, 0x00, 0x00,
0x00, 0x03, 0xFB, 0x80, 0x08, 0x00, 0x00, 0x12, 0x8D, 0x00, 0x07, 0x80, 0x38, 0x1E, 0x0F, 0x80,
0x00, 0x03, 0xFD, 0x80, 0x08, 0x00, 0x00, 0x0C, 0x06, 0x24, 0x80, 0x70, 0x00, 0x00, 0x08, 0x00,
0x00, 0x01, 0xFE, 0xC0, 0x08, 0x00, 0x00, 0x00, 0x08, 0x24, 0x80, 0xF1, 0x7C, 0x0C, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x1F, 0xF8, 0x00, 0x00, 0x5E, 0x06, 0x24, 0x87, 0x00, 0x00, 0x12, 0x0F, 0x00,
0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F, 0x83, 0xC0, 0x3C, 0x12, 0x08, 0x80,
0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x02, 0x0C, 0x00, 0x00, 0x70, 0x40, 0x0C, 0x07, 0x00,
0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x02, 0x04, 0x00, 0x81, 0xE0, 0x40, 0x04, 0x00, 0x00,
0x00, 0x01, 0xFF, 0x83, 0xC0, 0x02, 0xC0, 0x7E, 0x0D, 0x00, 0x86, 0x00, 0x3C, 0x1A, 0x0E, 0x80,
0x00, 0x07, 0xFF, 0x8C, 0x40, 0x03, 0x40, 0x00, 0x0D, 0x00, 0x80, 0x00, 0x40, 0x0A, 0x09, 0x80,
0x00, 0x06, 0x00, 0x0E, 0x40, 0x01, 0x80, 0x00, 0x06, 0x3F, 0x80, 0x00, 0x40, 0x00, 0x08, 0x80,
0x00, 0x0C, 0x00, 0x01, 0xE0, 0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x0E, 0x00, 0x00,
0x00, 0x0C, 0x00, 0x00, 0x38, 0x02, 0xC0, 0x00, 0x01, 0x1F, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
0x00, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x01, 0x31, 0x01, 0x00, 0x7C, 0x1E, 0x1F, 0x80,
0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0xF8, 0x3F, 0x20, 0x81, 0xF0, 0x04, 0x00, 0x00, 0x00,
0x00, 0x04, 0x00, 0x03, 0x00, 0x03, 0xC1, 0x04, 0x00, 0x20, 0x80, 0x00, 0x04, 0x0A, 0x0F, 0x80,
0x00, 0x03, 0x00, 0x00, 0xC0, 0x00, 0x02, 0x02, 0x00, 0x11, 0x00, 0xE0, 0x78, 0x1A, 0x04, 0x00,
0x01, 0xFF, 0xFF, 0x83, 0xA0, 0x03, 0xC2, 0x02, 0x08, 0x0E, 0x01, 0x10, 0x00, 0x1A, 0x07, 0x80,
0x01, 0xFF, 0xFF, 0x81, 0x80, 0x00, 0x42, 0x02, 0x0F, 0x00, 0x01, 0x11, 0xFC, 0x0C, 0x00, 0x00,
0x00, 0x00, 0x00, 0x02, 0x40, 0x03, 0xC1, 0x8C, 0x00, 0x00, 0x00, 0xE0, 0x20, 0x14, 0x07, 0x80,
0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x78, 0x05, 0x00, 0x00, 0x00, 0x20, 0x1A, 0x04, 0x80,
0x00, 0x00, 0x00, 0x0F, 0xC0, 0x02, 0x40, 0x00, 0x0D, 0x00, 0x01, 0x60, 0x20, 0x0A, 0x07, 0xE0,
0x00, 0x01, 0xFF, 0x80, 0x00, 0x07, 0x83, 0xFE, 0x06, 0x00, 0x01, 0x51, 0xFC, 0x00, 0x00, 0x00,
0x00, 0x07, 0xFF, 0x80, 0x00, 0x01, 0xC0, 0x0C, 0x00, 0x04, 0x00, 0x90, 0x00, 0x00, 0x01, 0x80,
0x00, 0x06, 0x00, 0x0F, 0xC0, 0x03, 0x40, 0x18, 0x09, 0x07, 0x80, 0x00, 0x00, 0x08, 0x0F, 0x00,
0x00, 0x0C, 0x00, 0x02, 0x40, 0x02, 0xC0, 0x60, 0x1E, 0x03, 0x00, 0xF0, 0x00, 0x1A, 0x0F, 0x00,
0x00, 0x0C, 0x00, 0x02, 0x40, 0x01, 0x80, 0xC0, 0x07, 0x04, 0x81, 0x00, 0x00, 0x1A, 0x01, 0x80,
0x00, 0x0C, 0x00, 0x01, 0xC0, 0x02, 0xC3, 0x80, 0x0D, 0x04, 0x81, 0x00, 0x00, 0x0C, 0x00, 0x00,
0x00, 0x04, 0x00, 0x01, 0x80, 0x02, 0xC3, 0xFE, 0x0D, 0x03, 0x01, 0xF0, 0x7E, 0x10, 0x00, 0x00,
0x00, 0x04, 0x00, 0x02, 0xC0, 0x01, 0x80, 0x00, 0x02, 0x01, 0x00, 0xC0, 0x45, 0x1E, 0x00, 0x00,
0x00, 0x03, 0x00, 0x02, 0xC0, 0x0A, 0x00, 0x00, 0x30, 0x04, 0x81, 0x50, 0x45, 0x00, 0x00, 0x00,
0x00, 0x0F, 0xFF, 0x81, 0x80, 0x07, 0xC3, 0xFC, 0x07, 0x06, 0x81, 0x50, 0x38, 0x1E, 0x00, 0x00,
0x00, 0x0F, 0xFF, 0x81, 0xC0, 0x00, 0x00, 0x06, 0x0E, 0x00, 0x00, 0x50, 0x00, 0x02, 0x00, 0x00,
0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x38, 0x03, 0x80, 0xE0, 0x3C, 0x02, 0x00, 0x00,
0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x07, 0x04, 0x00, 0x60, 0x40, 0x1C, 0x00, 0x00,
0x00, 0x00, 0xE0, 0x03, 0xC0, 0x00, 0x00, 0x04, 0x0E, 0x04, 0x04, 0x90, 0x40, 0x12, 0x00, 0x00,
0x00, 0x03, 0xE1, 0x03, 0xE0, 0x00, 0x03, 0xF8, 0x30, 0x07, 0x84, 0x90, 0x40, 0x3E, 0x00, 0x00,
0x00, 0x06, 0x21, 0x82, 0x50, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x10, 0x7C, 0x00, 0x00, 0x00,
0x00, 0x04, 0x21, 0x82, 0x50, 0x00, 0x00, 0x00, 0x00, 0x06, 0x80, 0x00, 0x00, 0x1E, 0x00, 0x00,
0x00, 0x0C, 0x21, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x06, 0x80, 0x01, 0x7C, 0x1A, 0x00, 0x00,
0x00, 0x08, 0x21, 0x87, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x16, 0x00, 0x00,
0x00, 0x0C, 0x21, 0x80, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x03, 0x00, 0x60, 0x44, 0x00, 0x00, 0x00,
0x00, 0x04, 0x21, 0x02, 0xC0, 0x00, 0x00, 0x42, 0x0F, 0x12, 0x80, 0x70, 0xFC, 0x10, 0x00, 0x00,
0x00, 0x06, 0x27, 0x03, 0x40, 0x00, 0x00, 0x42, 0x10, 0x14, 0x81, 0xC0, 0x40, 0x1E, 0x00, 0x00,
0x00, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x42, 0x10, 0x08, 0x80, 0x60, 0x00, 0x08, 0x00, 0x00,
0x00, 0x00, 0xF8, 0x03, 0xC0, 0x00, 0x00, 0x3C, 0x1F, 0x00, 0x00, 0x70, 0x3C, 0x1A, 0x00, 0x00,
0x00, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x40, 0x1A, 0x00, 0x00,
0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x0F, 0x02, 0x00, 0xE0, 0x40, 0x0C, 0x00, 0x00,
0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x15, 0x06, 0x81, 0x13, 0xFC, 0x00, 0x00, 0x00,
0x00, 0x03, 0xE1, 0x0F, 0xC0, 0x00, 0x00, 0x40, 0x13, 0x06, 0x81, 0x10, 0x00, 0x0C, 0x00, 0x00,
0x00, 0x06, 0x21, 0x82, 0x40, 0x00, 0x00, 0x40, 0x00, 0x03, 0x01, 0x10, 0x7E, 0x12, 0x00, 0x00,
0x00, 0x0C, 0x21, 0x83, 0xC0, 0x00, 0x00, 0x20, 0x48, 0x04, 0x00, 0xE0, 0x45, 0x12, 0x00, 0x00,
0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x7E, 0x48, 0x07, 0x80, 0x00, 0x45, 0x1F, 0x80, 0x00,
0x00, 0x08, 0x21, 0x80, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x07, 0xF0, 0x45, 0x0E, 0x00, 0x00,
0x00, 0x04, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x38, 0x10, 0x00, 0x00,
0x00, 0x06, 0x23, 0x00, 0x00, 0x00, 0x02, 0x7E, 0x00, 0x00, 0x84, 0x80, 0x00, 0x10, 0x00, 0x00,
0x00, 0x03, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x81, 0x7C, 0x0E, 0x00, 0x00,
0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x10, 0x07, 0x07, 0xF0, 0x00, 0x10, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x1F, 0x04, 0x80, 0x00, 0x04, 0x1E, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0F, 0x80, 0x00, 0x04, 0x00, 0x00, 0x00,
0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x5F, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x00, 0x00,
0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x01, 0x01, 0x01, 0x01, 0xFC, 0x1A, 0x00, 0x00,
0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x06, 0x81, 0xF0, 0x00, 0x1A, 0x00, 0x00,
0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x02, 0x80, 0x00, 0x00, 0x4C, 0x00, 0x00,
0x00, 0x0F, 0xFF, 0x80, 0x00, 0x00, 0x03, 0xFE, 0x74, 0x00, 0x00, 0xD0, 0x00, 0x40, 0x00, 0x00,
0x00, 0x0F, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x64, 0x0E, 0x17, 0x81, 0x50, 0x70, 0x7E, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x01, 0x03, 0x01, 0x50, 0x88, 0x40, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x04, 0x80, 0xE1, 0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x04, 0x81, 0x11, 0x04, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0xF1, 0x04, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x01, 0xFC, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x1C, 0x00, 0xF1, 0x24, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xC1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x51, 0x24, 0x00, 0x00, 0x00,
0x00, 0xC0, 0x81, 0x80, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x01, 0x31, 0x24, 0x00, 0x00, 0x00,
0x00, 0xC0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x1C, 0x04, 0x01, 0xFC, 0x00, 0x00, 0x00,
0x00, 0xC0, 0x01, 0x80, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x0F, 0x81, 0xC0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x40, 0x01, 0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x00, 0x00, 0x70, 0x04, 0x00, 0x00, 0x00,
0x00, 0x60, 0x03, 0x00, 0x00, 0x00, 0x01, 0xD0, 0x00, 0x00, 0x07, 0x80, 0x04, 0x00, 0x00, 0x00,
0x00, 0x30, 0x07, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x07, 0x80, 0x04, 0x00, 0x00, 0x00,
0x00, 0x1C, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x71, 0xFC, 0x00, 0x00, 0x00,
0x00, 0x0F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
=====================================================
|| Initialize OLED display and show startup image. ||
===================================================== */
void Display::viewStartupImage() {
  Serial.println("STARTUP_IMAGE");

  /*
  //Startup image.
  SeeedGrayOled.drawBitmap(greenhouse, 128*128/8);  //Show greenhouse logo. Second parameter in drawBitmap function specifies the size of the image in bytes. Fullscreen image = 128 * 64 pixels / 8.
  delay(4000);                                      //Image shown for 4 seconds.
  SeeedGrayOled.clearDisplay();                     //Clear the display.
  */

  displayState = SET_CLOCK;                         //Set next display mode to be printed to display.
}
