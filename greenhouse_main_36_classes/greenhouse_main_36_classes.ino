/*
*************************
* Included header files *
*************************/
#include "Wire.h"
#include "SeeedOLED.h"
#include "DHT.h"
#include "SI114X.h"
#include "Moisture.h"
#include "Temperature.h"
#include "Lighting.h"
#include "Watering.h"
#include "Display.h"
#include "Miscellaneous.h"

//Moisture sensors.
Moisture moistureSensor1;                 //Moisture sensor1 created from Moisture class.
Moisture moistureSensor2;                 //Moisture sensor2 created from Moisture class.
Moisture moistureSensor3;                 //Moisture sensor3 created from Moisture class.
Moisture moistureSensor4;                 //Moisture sensor4 created from Moisture class.
Moisture moistureSensor;                  //Fictional moisture sensor that holds moisture mean value. Created from the Moisture class.

//Temperature and humidity sensor.
const uint8_t DHTTYPE = DHT11;            //DHT11 = Arduino UNO model is being used.
DHT humiditySensor(DHTPIN, DHTTYPE);      //Humidity sensor object created from DHT class.
Temperature tempRotaryEncoder;            //Rotary encoder object created from Temperature class.

//Light sensor and LED lights.
SI114X lightSensor;                       //Light sensor object created from SI114X class.
Lighting ledLights;                       //LED lights object created from Lighting class.

//Water pump, flow sensor and water level sensor.
Watering waterPump;                       //Water pump object created from Water class.
 
//OLED display.
Display oledDisplay;                      //OLED display object created from Display class.



//Debouncing button press on button connected to external interrupt.
volatile unsigned long pressTimePrev;     //Variable to store previous millis() value.
int debounceTimePeriod = 170;             //Delay time before interrupt function is started.


//Internal clock to keep track of current time.
int hourPointer1 = 0;
int hourPointer2 = 0;
int minutePointer1 = 0;                 //1-digit of minute pointer.
int minutePointer2 = 0;                 //10-digit of minute pointer.
int secondPointer1 = 0;                 //1-digit of second pointer.
int secondPointer2 = 0;                 //10-digit of second pointer.
bool hour2InputMode = true;
bool hour1InputMode = false;
bool minute2InputMode = false;
bool minute1InputMode = false;
bool pushButton1 = false;


bool clockStartMode = false;
bool clockSetFinished = false;
                          
bool flashClockPointer = false;         //Variable to create lash clock pointer when in "set clock" mode.       
int x = 0;                              //Toggle variable to flash current clock pointer.

//Alarm messages to display.
bool alarmMessageEnabled = false;       //Enable any alarm to be printed to display. If variable is 'true' alarm is enable to be printed to display.
unsigned long alarmTimePrev = 0;        //Used to read relative time 
unsigned long alarmTimePeriod = 2100;   //Variable value specifies in milliseconds, for how long time each warning message will be shown on display before cleared and/or replaced by next warning message.

//Toggle display modes.
bool startupImageDisplay = true;        //Any variable is set to 'true' when that screen mode is currently printed to display.
bool setTimeDisplay = false;                 
bool readoutValuesDisplay = false;
bool serviceModeDisplay = false;
bool clockViewFinished = false;
bool flowFaultDisplay = false;

/*
---------------------
|Greenhouse program.|
--------------------*/
bool greenhouseProgramStart = false;        //If variable is set to 'true', automatic water and lighting control of greenhouse is turned on.

//Water pump.
unsigned int checkMoisturePeriod = 30000;        //Loop time, in milliseconds, for how often water pump is activated based upon measured soil moisture value.         
unsigned long checkMoistureStart = 0;
unsigned int checkWaterFlowPeriod = 1500;
unsigned long checkWaterFlowStart = 0;    
unsigned int waterPumpTimePeriod = 3000;  //Sets the time for how long water pump will run each time it is activated.
unsigned long waterPumpTimeStart = 0; 
int y = 0;                                  //Toggle variable to clear/set fault variable.

//LED lighting.
unsigned int checkLightNeedPeriod = 10000;  //Loop time for how often measured light value is checked. This enables/disables start of LED lighting.
unsigned long checkLightNeedStart = 0;
unsigned int checkLightFaultPeriod = 5000;  //Delay time after LED lighting has been turned ON, before checking if it works.
unsigned long checkLightFaultStart = 0;
bool insideTimeInterval = false;










/*
=========================================================================
|| VALUE READ OUT DISPLAY MODE. Print read out values to OLED display. ||
========================================================================= */
/*
void viewReadoutValues() {
  //Clear redundant value digits from previous read out for all sensor values.
  SeeedOled.setTextXY(0, 42);
  SeeedOled.putString("      ");
  SeeedOled.setTextXY(1, 42);
  SeeedOled.putString("      ");
  SeeedOled.setTextXY(1, 37);
  SeeedOled.putString("     ");  
  SeeedOled.setTextXY(2, 42);
  SeeedOled.putString("      ");    
  SeeedOled.setTextXY(3, 39);
  SeeedOled.putString("         ");
  SeeedOled.setTextXY(4, 42);
  SeeedOled.putString("    ");
  SeeedOled.setTextXY(4, 44);               //Clear symbols from previous display mode.
  SeeedOled.putString("    ");
  SeeedOled.setTextXY(5, 42);
  SeeedOled.putString("      ");

  //Printing read out values from the greenhouse to display.
  ************************
  |Moisture sensor value.|
  ************************/
 /* SeeedOled.setTextXY(0, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Moisture: ");        //Print string to display.
  SeeedOled.setTextXY(0, 42);
  SeeedOled.putNumber(moistureMeanValue);   //Print mean moisture value to display.

  //Select which text string to be printed to display depending of soil moisture.
  SeeedOled.setTextXY(0, 46);
  if(moistureDry == true && moistureWet == false) {
    SeeedOled.putString("Lo");             //Print string to display.
  }
  else if(moistureDry == false && moistureWet == false) {
    SeeedOled.putString("OK");             //Print string to display.
  }
  else if(moistureDry == false && moistureWet == true) {
    SeeedOled.putString("Hi");             //Print string to display.
  }  

  /*******************
  |Temp sensor value.|
  ********************/
  /*SeeedOled.setTextXY(1, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Temp: ");            //Print string to display.
  SeeedOled.setTextXY(1, 42);
  SeeedOled.putNumber(tempValue);           //Print temperature value to display.

  /***********************
  |UV-light sensor value.|
  ************************/
  /*SeeedOled.setTextXY(2, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("UV-light: ");        //Print string to display.
  SeeedOled.setTextXY(2, 42);
  SeeedOled.putNumber(uvValue);             //Print UV-light value to display.

  /********************
  |Light sensor value.|
  *********************/
  /*SeeedOled.setTextXY(3, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Light: ");           //Print string to display.
  SeeedOled.setTextXY(3, 42);
  SeeedOled.putNumber(lightValue);          //Print light value in the unit, lux, to display.
  SeeedOled.putString("lm");                 //Print unit of the value.

  /**********************
  |Temp threshold value.|
  ***********************/
  /*SeeedOled.setTextXY(4, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Temp lim: ");        //Print string to display.
  SeeedOled.setTextXY(4, 42);
  SeeedOled.putNumber(tempThresholdValue / 2);    //Print temperature threshold value to display. Value 24 corresponds to 12°C, temp value is doubled to reduce rotary sensitivity and increase knob rotation precision.
  
  /*************************
  |Water flow sensor value.|
  **************************/
  /*SeeedOled.setTextXY(5, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Flow Sens: ");       //Print string to display.
  SeeedOled.setTextXY(5, 42);
  SeeedOled.putNumber(waterFlowValue);      //Print water flow value to display.
  SeeedOled.putString("L/h");               //Print unit of the value.

  //Printing separator line to separate read out values from error/warning messages.
  SeeedOled.setTextXY(6, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("----------------");  //Print string to display.
}*/





/*
=======================================================================================================================================================
|| Timer interrupt to read temperature threshold value adjustments (set by rotary encoder) and to make pointer digits flash when in "set clock mode" ||
======================================================================================================================================================= */
ISR(TIMER2_COMPA_vect) {  //Timer interrupt with a frequency of 100Hz to read temperature threshold value set by rotary encoder.
  //Reading preset temperature threshold thas is being adjusted by rotary encoder knob.
  divider10++;
  if(divider10 == 10) {                                               //Divides the signal by 10 to make the function update set temperature (adjusted by the rotary encoder) with a frequency of 10Hz.
    divider10 = 0;
    int aState;
    aState = digitalRead(rotaryEncoderOutpA);                         //Reads the current state of the rotary knob, outputA.
  
    if(aState != aLastState) {                                        //A turn on rotary knob is detected by comparing previous and current state of outputA.
      if(digitalRead(rotaryEncoderOutpB) != aState && tempThresholdValue <= TEMP_VALUE_MAX) { //If outputB state is different to outputA state, that meant the encoder knob is rotation clockwise.
        tempThresholdValue++;                                         //Clockwise rotation means increasing position value. Position value is only increased if less than max value.
      }
      else if(tempThresholdValue > TEMP_VALUE_MIN) {
        tempThresholdValue--;                                         //Counter clockwise rotation means decreasing position value.
      }
    }
  aLastState = aState;                                                //Updates the previous state of outputA with current state.
  }
  
  //Flash clock pointer digits with a frequency of 2Hz.
  divider50++;                        
  if(divider50 == 50) {                                       //Divides the signal by 50 to give a 2Hz pulse to feed the flashing of pointer digits when in clock set mode.
    divider50 = 0;                                            //Reset divider variable.
    x++;
    if(x == 1) {
      flashClockPointer = true;
    }
    else {
      flashClockPointer = false;
      x = 0;
    }
  }
} 
  
/*
==================================================================
|| Timer interrupt to feed clock function with a signal of 1Hz. ||
================================================================== */
ISR(TIMER1_COMPA_vect) {  //Timer interrupt with a frequency of 1Hz to feed internal clock with signal to increment its second pointer.
  //Internal clock used to keep track of current time. This function will be run once every second.
  if(clockStartMode == true) {          //When set 'true' clock starts ticking.                       
      
    //Second pointer1.
    if(secondPointer1 == 10) {        //If 1-digit second pointer reaches a value of 10 (elapsed time is 10 seconds).
      secondPointer2++;               //Increase 10-digit second pointer.
      secondPointer1 = 0;             //Clear 1-digit pointer.
    }
    if(secondPointer2 == 6) {         //If 10-digit pointer reaches a value of 6 (elapsed time is 60 seconds).
      minutePointer1++;               //Increase minute pointer.
      secondPointer2 = 0;             //Clear 10-digit second pointer.
    }
    //Minute pointer.
    if(minutePointer1 == 10) {        //If 1-digit minute pointer reaches a value of 10 (elapsed time is 10 minutes).
      minutePointer2++;               //Increase 10-digit minute pointer.
      minutePointer1 = 0;             //Clear 1-digit minute pointer.
    }
    if(minutePointer2 == 6) {         //If 10-digit minute pointer reaches a value of 6 (elapsed time is 60 minutes).
      hourPointer1++;                 //Increase 1-digit hour pointer.
      minutePointer2 = 0;              //Clear 10-digit minute pointer.
    }
    //Hour pointer.
    if(hourPointer1 == 10) {          //If 1-digit hour pointer reaches a value of 10 (elapsed time is 10 hours).
      hourPointer2++;                 //Increase 10-digit hour pointer.
      hourPointer1 = 0;               //Clear 1-digit hour pointer.
    }
    if(hourPointer2 == 2 && hourPointer1 == 4) {  //If 1-digit and 10-digit hourPointer combined reaches 24 (elapsed time is 24 hours).
      hourPointer1 = 0;                           //Clear both hour digits.
      hourPointer2 = 0;
    }

    //Convert clock pointer into single int variable. Value of this variable represent clock time.                
    currentClockTime = 0;
    currentClockTime += (hourPointer2 * 1000);
    currentClockTime += (hourPointer1 * 100);
    currentClockTime += (minutePointer2 * 10);
    currentClockTime += minutePointer1;

    if(currentClockTime < 60) {           //Prevent clock time from seeing 00:00 as less than 23:00.       
      currentClockTime += 2400;
    }
    //Serial.println(currentClockTime);
  }
}

/*
===============================================================
|| Set current time by using SET- and MODE-buttons as input. ||
=============================================================== */
void setClockTime() {
  //Set current clock time by toggling each hour pointer and minute pointer individualy.
  if(pushButton1 == true && hour2InputMode == true) {
    delay(170);                                                 //Delay to avoid contact bounce.
    hourPointer2++;                                             //Increase hour pointer every time button is pressed.
    if(hourPointer2 == 3) {                                     //If 10-digit hour pointer reaches 3, clear digit.
      hourPointer2 = 0;
    }
  }
  else if(pushButton1 == true && hour1InputMode == true) {
    delay(170);                                                 //Delay to debounce button press.
    hourPointer1++;                                             //Increase hour pointer every time button is pressed.

    if(hourPointer2 == 2) {                                     //If hour pointer2 is equal to 2, hour pointer 1 is only allowed to reach a maximum value of 4.
      if(hourPointer1 == 5) {
        hourPointer1 = 0;
      }
    }
    else {
      if(hourPointer1 == 10) {                                  //If 1-digit hour pointer reaches 10, clear digit.
      hourPointer1 = 0;
      }
    }
  }
  else if(pushButton1 == true && minute2InputMode == true) {
    delay(170);                                                 
    minutePointer2++;
    if(minutePointer2 == 6) {                                   //If 10-digit minute pointer reaches a value of 6, clear 10-digit minute pointer.
      minutePointer2 = 0;    
    }
  }
  else if(pushButton1 == true && minute1InputMode == true) {
    delay(170);                                                 
    minutePointer1++;
    if(minutePointer1 == 10) {                                 //If 10-digit minute pointer reaches a value of 10, clear 1-digit minute pointer.
      minutePointer1 = 0;    
    }
  }

  //Replace clock time represenation. When current clock time is 24 hours is replaced with 00.
  if(clockStartMode == true) {
    if(hourPointer2 == 2 && hourPointer1 == 4) {                //If 10-digit hour pointer reaches a value of 2 and 1-digit hour pointer reaches a value of 4 (elapsed time is 24 hours).
      hourPointer2 = 0;                                         //Clear both hour pointer values.
      hourPointer1 = 0;
    }
  }
}

/*
======================
|| Reset clock time ||
====================== */
void resetClockTime() {
  //Stop clock and reset all clock pointers.
  clockStartMode = false;                       //Stop clock from ticking.
  hourPointer2 = 0;                             //Reset all clock pointers individualy.
  hourPointer1 = 0;
  minutePointer2 = 0;
  minutePointer1 = 0; 
  secondPointer2 = 0;                           
  secondPointer1 = 0; 
  Serial.println("resetClockTime");                                           
}

/*
======================================================================================
|| Toggle set modes and screen display modes when clockModeButton is being pressed. ||
====================================================================================== */
void toggleDisplayMode() {
  //Debouncing button press to avoid multiple interrupts, display toggles.
  if((millis() - pressTimePrev) >= debounceTimePeriod) {

    //Toggle display modes every time MODE-button is pressed.
    if(setTimeDisplay == true) {
      Serial.println("setTimeDisplay");
      if(hour2InputMode == true) {
        hour2InputMode = false;                 //Hour pointer2 has been set.
        hour1InputMode = true;                  //Continue by setting hour pointer1.
        Serial.println("hour2InputMode");
      }
      else if(hour1InputMode == true) {
        hour1InputMode = false;                 //Hour pointer1 has been set.
        minute2InputMode = true;                //Continue by setting minute pointer2.
        Serial.println("hour1InputMode");
      }
      else if(minute2InputMode == true) {
        minute2InputMode = false;               //Minute pointer2 has been set.
        minute1InputMode = true;                //Continue by setting minute pointer1.
        Serial.println("minute2InputMode");
      }
      else if(minute1InputMode == true) {
        minute1InputMode = false;               //Minute pointer1 has been set. Time set is done.
        clockStartMode = true;                  //Start clock. Clock starts ticking.
        clockSetFinished = true;
        Serial.println("minute1InputMode");
      }
      else if(clockSetFinished == true) {
        clockSetFinished = false;               //Clear current state in display mode.
        setTimeDisplay = false;                 //Clear current display mode.
        readoutValuesDisplay = true;            //Set next display mode to be printed to display.
        alarmMessageEnabled = true;             //Enable any alarm message to be printed to display.
        greenhouseProgramStart = true;          //Start greenhouse program.
        Serial.println("clockSetFinished");
      }
    }
    else if(readoutValuesDisplay == true) {
      readoutValuesDisplay = false;               //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      alarmMessageEnabled = false;                //Disable any alarm message from being printed to display.
      //SeeedOled.clearDisplay();                   //Clear display.
      serviceModeDisplay = true;                  //Set next display mode to be printed to display.
      Serial.println("readoutValuesDisplay");
    }
    else if(serviceModeDisplay == true) {
      serviceModeDisplay = false;                 //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      //SeeedOled.clearDisplay();                   //Clear display.
      readoutValuesDisplay = true;                //Set next display mode to be printed to display.
      alarmMessageEnabled = true;                 //Enable any alarm message from being printed to display.
      Serial.println("serviceModeDisplay");
    }
    else if(flowFaultDisplay == true) {
      flowFaultDisplay = false;                   //Clear current screen display mode to enable next display mode to shown next time MODE-button is pressed.
      //SeeedOled.clearDisplay();                   //Clear display.
      Serial.println("flowFaultDisplay");
      
      //Check if water flow fault code has been cleared by user or not.
      if(waterFlowFault == false) {               //Fault code has been cleared by user. Continue to run greenhouse program.               
        greenhouseProgramStart = true;            
        readoutValuesDisplay = true;              //Set next display mode to be printed to display.
        alarmMessageEnabled = true;               //Enable any alarm message from being printed to display.
        Serial.println("Resume program"); 
      }
      else if(waterFlowFault == true) {           //If flow fault code is not cleared, reboot greenhouse program.
        waterFlowFault = false;                   //Clear water flow fault code.
        resetClockTime();                         //Reset clock time.
        startupImageDisplay = true;               //Reboot greenhouse program by printing the startup image to display.                
        Serial.println("Go to startupImageDisplay");             
      }
    }

    //Check if water flow fault code is active. If active enter flow fault display to handle fault code.
    if(waterFlowFault == true) {
      readoutValuesDisplay = false;               //Clear any of current screen display modes to enable next display mode to shown next time MODE-button is pressed.
      serviceModeDisplay = false;
      alarmMessageEnabled = false;
      flowFaultDisplay = true;                    //Set next display mode to be printed to display.
      greenhouseProgramStart = false;             //Stop greenhouse program.
      waterPump.stopPump();                       //Stop water pump.
    }
  pressTimePrev = millis();
  }
}

/*
===================================================================================================
|| SET CLOCK TIME DISPLAY MODE. Print clock values to OLED display to let user set current time. ||
=================================================================================================== */
void setClockDisplay() {
  SeeedOled.setTextXY(0, 0);                            //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Set current time");              //Print text to display.
  SeeedOled.setTextXY(1, 0);                            //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Use buttons:");                  
  SeeedOled.setTextXY(2, 0);                            
  SeeedOled.putString("SET = inc. p.val");              
  SeeedOled.setTextXY(3, 0);                            
  SeeedOled.putString("MODE = h or min");               
  SeeedOled.setTextXY(5, 20);                           
  SeeedOled.putString("HH");                            
  SeeedOled.setTextXY(5, 23);                           
  SeeedOled.putString("MM");                            
  SeeedOled.setTextXY(5, 26);                           
  SeeedOled.putString("SS");                            

  //Flashing individual clock time pointers to display which clock parameter that is currently set.
  //Hour pointer2
  if(flashClockPointer == true && hour2InputMode == true) {
    SeeedOled.setTextXY(6, 20);                           
    SeeedOled.putString(" ");                             //Clear display where 10-digit hour pointer value is located.    
  }
  else {
    SeeedOled.setTextXY(6, 20);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedOled.putNumber(hourPointer2);                    //Print 10-digit hour pointer value to display.
  }
  
  //Hour pointer1.
  if(flashClockPointer == true && hour1InputMode == true) {
    SeeedOled.setTextXY(6, 21);                           
    SeeedOled.putString(" ");                             //Clear display where 1-digit hour pointer value is located.

  }
  else {
    SeeedOled.setTextXY(6, 21);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedOled.putNumber(hourPointer1);                    //Print 1-digit hour pointer value to display.
  }

  //Pointer separator symbol.
  SeeedOled.setTextXY(6, 22);                             //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString(":");                               //Print symbol to display.

  //Minute pointer2.
  if(flashClockPointer == true && minute2InputMode == true) {
    SeeedOled.setTextXY(6, 23);                           
    SeeedOled.putString(" ");    
  }                  
  else {
    SeeedOled.setTextXY(6, 23);                           
    SeeedOled.putNumber(minutePointer2);
  }

  //Minute pointer1.
  if(flashClockPointer == true && minute1InputMode == true) {
    SeeedOled.setTextXY(6, 24);                           
    SeeedOled.putString(" ");                
  }
  else {
    SeeedOled.setTextXY(6, 24);                           
    SeeedOled.putNumber(minutePointer1);   
  }

  //Pointer separator symbol.
  SeeedOled.setTextXY(6, 25);                           
  SeeedOled.putString(":");

  //Second pointers.
  SeeedOled.setTextXY(6, 26);                           
  SeeedOled.putNumber(secondPointer2);                   //Print second digit of second pointer value to display.
  SeeedOled.setTextXY(6, 27);                           
  SeeedOled.putNumber(secondPointer1);                   //Print first digit of second pointer value to display.
}

/*
============================================================================================================
|| ALARM MESSAGE TO DISPLAY. Print alarm message to OLED display for any fault that is currently active . ||
============================================================================================================ */
void alarmMessageDisplay() {       
  unsigned long alarmTimeNow;
  unsigned long alarmTimeDiff;                            //Calculate time difference from when the warning message function was started. Keeps track for how long time each warning message is shown on display.
                                                              
  //Print multiple warning messages to display, using the same space of display. One alarm message after another.
  alarmTimeNow = millis();                                //Read millis() value to get the time stamp when first alarm message was printed to display.                      
  alarmTimeDiff = alarmTimeNow - alarmTimePrev;
  //Serial.print("alarmTimeDiff: ");
  //Serial.println(alarmTimeDiff);

  if(alarmMessageEnabled == true) {                       //Any alarm can only be printed to display if variable is set to 'true'.
    /******************
    |Water flow fault.|
    *******************/
    if(alarmTimeDiff <= alarmTimePeriod) {
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
      SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      if(waterFlowFault == true) {                        //If fault variable is set to 'true', fault message is printed to display.
        SeeedOled.setTextXY(7, 0);                        
        SeeedOled.putString("No water flow");             //Print fault message to display.
      }

      else {  //If this alarm not active, clear the warning message row.
        SeeedOled.setTextXY(7, 0);                        //Set cordinates to the warning message will be printed.
        SeeedOled.putString("                ");          //Clear row to enable other warnings to be printed to display.
      }
    }

    /**********************
    |Low water tank level.|
    ***********************/
    if(alarmTimePeriod < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 2) {
      if(waterLevelFault == true) {                         //If fault variable is set to 'true', fault message is printed to display.
        SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
        SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedOled.setTextXY(7, 0);                          
        SeeedOled.putString("Low water level");             //Print fault message to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
        SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.      
      }
    }

    /*******************
    |Temperature fault.|
    ********************/
    if(alarmTimePeriod * 2 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 3) { 
      if(tempValueFault == true) {                          //If fault variable is set to 'true', fault message is printed to display.
        SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
        SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedOled.setTextXY(7, 0);                          
        SeeedOled.putString("High temperature");            //Print fault message to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
        SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.      
      }
    }

    /********************
    |LED lighting fault.|
    *********************/
    if(alarmTimePeriod * 3 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 4) { 
      if(ledLightFault == true) {                           //If fault variable is set to 'true', fault message is printed to display.
        SeeedOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
        SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedOled.setTextXY(7, 0);                          
        SeeedOled.putString("LED not working");             //If measured water flow is below a certain value without the water level sensor indicating the water tank is empty, there is a problem with the water tank hose. "Check water hose!" is printed to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
        SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.      
      }
    }
  
    if(alarmTimeDiff > alarmTimePeriod * 4) {
      SeeedOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      alarmTimePrev = millis();                           //Read millis() value to reset time delay calculation.
    }  
  }
}

/*
===========================================================================
|| SERVICE MODE DISPLAY MODE. Print service mode screen to OLED display. ||
=========================================================================== */
/*
void viewServiceMode() {
  //Clear redundant value digits from previous read out for all sensor values.
  SeeedOled.setTextXY(0, 39);
  SeeedOled.putString(" ");
  SeeedOled.setTextXY(1, 25);  
  SeeedOled.putString("     ");  
  SeeedOled.setTextXY(2, 19);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(2, 24);                            
  SeeedOled.putString(" ");  
  SeeedOled.setTextXY(2, 28);
  SeeedOled.putString("   ");     
  SeeedOled.setTextXY(3, 19);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(3, 28);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(4, 27);
  SeeedOled.putString(" ");
  SeeedOled.setTextXY(5, 26);
  SeeedOled.putString("  ");
  SeeedOled.setTextXY(5, 45);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(6, 42);
  SeeedOled.putString("  ");
  SeeedOled.setTextXY(6, 45);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(7, 45);
  SeeedOled.putString("  ");
  
  //Display clock.
  SeeedOled.setTextXY(0, 0);                            //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Clock: ");                       //Print string to display.
  //Hour pointer.
  SeeedOled.setTextXY(0, 40);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(hourPointer2);                    //Print 10-digit hour pointer value to display.
  SeeedOled.setTextXY(0, 41);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(hourPointer1);                    //Print 1-digit hour pointer value to display.
  
  SeeedOled.setTextXY(0, 42);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString(":");                             //Print separator symbol, between hour and minute digits, todisplay.

  //Minute pointer.
  SeeedOled.setTextXY(0, 43);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(minutePointer2);                  //Print 10-digit hour pointer value to display.
  SeeedOled.setTextXY(0, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(minutePointer1);                  //Print 1-digit hour pointer value to display.

  SeeedOled.setTextXY(0, 45);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putString(":");                             //Print separator symbol, between hour and minute digits, todisplay.

  //Second pointer.
  SeeedOled.setTextXY(0, 46);                           
  SeeedOled.putNumber(secondPointer2);                  //Print second digit of second pointer value to display.
  SeeedOled.setTextXY(0, 47);                           
  SeeedOled.putNumber(secondPointer1);                  //Print first digit of second pointer value to display.

  //Display moisture sensor values.
  SeeedOled.setTextXY(1, 0);                            //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedOled.putString("Moisture:");                     //Print text to display.
  SeeedOled.setTextXY(2, 0);                            
  SeeedOled.putString("S1[");                           //Print text to display.
  SeeedOled.setTextXY(2, 19);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue1);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(2, 22);                            
  SeeedOled.putString("],");                     

  SeeedOled.setTextXY(2, 25);                            
  SeeedOled.putString("S2[");                           //Print text to display.
  SeeedOled.setTextXY(2, 28);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue2);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(2, 31);                            
  SeeedOled.putString("]");
  
  SeeedOled.setTextXY(3, 0);                            
  SeeedOled.putString("S3[");                           //Print text to display.
  SeeedOled.setTextXY(3, 19);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue3);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(3, 22);                            
  SeeedOled.putString("],");                     

  SeeedOled.setTextXY(3, 25);                            
  SeeedOled.putString("S4[");                           //Print text to display.
  SeeedOled.setTextXY(3, 28);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(moistureValue4);                  //Print moisture sensor1 value.
  SeeedOled.setTextXY(3, 31);                            
  SeeedOled.putString("]");

  //Alarm messsage status.
  SeeedOled.setTextXY(4, 0);                            
  SeeedOled.putString("tempValue: ");                   //Print text to display.
  SeeedOled.setTextXY(4, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(tempValueFault);                  //Print moisture sensor1 value.

  SeeedOled.setTextXY(5, 0);                            
  SeeedOled.putString("ledLight: ");                    //Print text to display.
  SeeedOled.setTextXY(5, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(ledLightFault);                   //Print moisture sensor1 value.

  SeeedOled.setTextXY(6, 0);                            
  SeeedOled.putString("waterFlow: ");                   //Print text to display.
  SeeedOled.setTextXY(6, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(waterFlowFault);                  //Print moisture sensor1 value.

  SeeedOled.setTextXY(7, 0);                            
  SeeedOled.putString("waterLevel: ");                  //Print text to display.
  SeeedOled.setTextXY(7, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedOled.putNumber(waterLevelFault);                 //Print moisture sensor1 value.
}
*/

/*
=========================================================================
|| FLOW FAULT DISPLAY MODE. Print service mode screen to OLED display. ||
========================================================================= */
bool resolveFlowFault() {    
  bool fault;
  
  //Clear redundant symbols from previous screen mode.
  SeeedOled.setTextXY(0, 28);
  SeeedOled.putString("    ");
  SeeedOled.setTextXY(2, 45);
  SeeedOled.putString("   ");
  SeeedOled.setTextXY(5, 45);
  SeeedOled.putString("   ");
  
  //Print fault code instruction to display. To let user resolve fault.
  SeeedOled.setTextXY(0, 0);
  SeeedOled.putString("waterFlow: ");
  SeeedOled.setTextXY(0, 27);
  SeeedOled.putNumber(waterFlowFault);
  
  SeeedOled.setTextXY(1, 0);
  SeeedOled.putString("----------------");
  SeeedOled.setTextXY(2, 0);
  SeeedOled.putString("Chk hardware: ");
  SeeedOled.setTextXY(3, 0);
  SeeedOled.putString("*is hose empty?");
  SeeedOled.setTextXY(4, 0);
  SeeedOled.putString("*vacuum in tank?");
  SeeedOled.setTextXY(5, 0);
  SeeedOled.putString("*Done? Press:");
  SeeedOled.setTextXY(6, 0);
  SeeedOled.putString("SET = clear/set");
  SeeedOled.setTextXY(7, 0);
  SeeedOled.putString("MODE = confirm");

  //Clear fault code status.
  if(pushButton1 == true && y == 0) {
    fault;                                      //Clear fault code to let water pump run.
    y = 1;                                      
    Serial.println("waterFlowFault inactive");
  }
  else if(pushButton1 == true && y == 1) {
    fault = true;                               //Let fault code stay active.
    y = 0;
    Serial.println("waterFlowFault active");
  }
  return fault;                                 //Return current fault status.
}

/*
*******************************
* Arduino program setup code. *
*******************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(SENSOR_PORT1, INPUT);
  pinMode(SENSOR_PORT2, INPUT);
  pinMode(SENSOR_PORT3, INPUT);
  pinMode(SENSOR_PORT4, INPUT);
  
  pinMode(flowSensor, INPUT);
  attachInterrupt(1, Watering::flowCount, RISING);  //Initialize interrupt to enable water flow sensor to calculate water flow pumped by water pump.

  attachInterrupt(0, toggleDisplayMode, RISING);    //Initialize interrupt to toggle set modes when in clock set mode or toggling screen display mode when greenhouse program is running. Interrupt is triggered by clockModeButton being pressed.
  
  pinMode(pumpRelay, OUTPUT);
  //pinMode(pumpButton, INPUT);
  
  pinMode(waterLevelSwitch, INPUT);
  
  pinMode(lightRelay, OUTPUT);
  
  pinMode(rotaryEncoderOutpA, INPUT);
  pinMode(rotaryEncoderOutpB, INPUT);
  aLastState = digitalRead(rotaryEncoderOutpA);      //Read initial position value.

  pinMode(clockSetButton, INPUT);
  pinMode(clockModeButton, INPUT);
  
  humiditySensor.begin();                 //Initializing humidity sensor.
  
  lightSensor.Begin();                    //Initializing light sensor.

  
  cli();                    //Stop interrupts.

  //Timer1 setup with interrupt frequency of 1Hz to feed internal clock.
  TCCR1A = 0;                             //Set entire TCCR1A register to 0.
  TCCR1B = 0;                             //Set entire TCCR1B register to 0.
  TCNT1 = 0;                              //Initialize counter value to 0.
  OCR1A = 15624;                          //match reg. = (16MHz / (prescaler * desired interrupt freq.)) - 1 = (16000000 / (1024 * 1)) - 1 = 15624 (must be < 65536).
  TCCR1B |= (1 << WGM12);                 //Turn on CTC mode.
  TCCR1B |= (1 << CS12) | (1 << CS10);    //Set CS10 and CS12 bits for 1024 prescaler.
  TIMSK1 |= (1 << OCIE1A);                //Enable timer compare interrupt.

  //Timer2 setup with interrupt frequency of 100Hz read temperature threshold value adjustmenst by rotary encoder.
  TCCR2A = 0;                             //Set entire TCCR0A register to 0.
  TCCR2B = 0;                             //Set entire TCCR0B register to 0.
  TCNT2 = 0;                              //Initialize counter value to 0.
  OCR2A = 156;                            //match reg. = (16MHz / (prescaler * desired interrupt freq.)) - 1 = (16000000 / (1024 * 100)) - 1 = 156.25 (must be < 256).
  TCCR2A |= (1 << WGM21);                 //Turn on CTC mode.
  TCCR2B |= (1 << CS21);                  //Set CS21 for 1024 prescaler.
  TIMSK2 |= (1 << OCIE2A);                //Enable timer compare interrupt.
  
  sei();                    //Allow interrupts again.
}

/*
******************************************
* Arduino program main code to be looped *
******************************************/
void loop() {
  // put your main code here, to run repeatedly:
  unsigned short moistureValue1;                                    //Moisture sensor value from moisture sensor 1.
  unsigned short moistureValue2;                                    //Moisture sensor value from moisture sensor 2.
  unsigned short moistureValue3;                                    //Moisture sensor value from moisture sensor 3.
  unsigned short moistureValue4;                                    //Moisture sensor value from moisture sensor 4.                      
  unsigned short moistureMeanValue;                                 //Mean value from moisture readouts for all 4 moisture sensors.

  moistureValue1 = moistureSensor1.readValue(SENSOR_PORT1);         //Read moistureSensor1 value to check soil humidity.
  moistureValue2 = moistureSensor2.readValue(SENSOR_PORT2);         //Read moistureSensor2 value to check soil humidity.
  moistureValue3 = moistureSensor3.readValue(SENSOR_PORT3);         //Read moistureSensor3 value to check soil humidity.
  moistureValue4 = moistureSensor4.readValue(SENSOR_PORT4);         //Read moistureSensor4 value to check soil humidity.

  moistureMeanValue = moistureSensor.calculateMean(moistureValue1, moistureValue2, moistureValue3, moistureValue4);   //Calculate moisture mean value from all 4 moisture sensors.
  
  moistureSensor.evaluateValue(moistureMeanValue, &moistureDry, &moistureWet);                                        //Set and/or clear the internal fault code variables: moistureDry and moistureWet. Fault code is active when set to 'true'.

  unsigned short tempValue;                                         //Readout temperature value.
  unsigned short humidValue;                                        //Readot air humidity value.
  tempValue = humiditySensor.readTemperature(false);                //Read temperature.
  //humidValue = humiditySensor.readHumidity();                     //Read air humidity.
  tempValueFault = tempRotaryEncoder.thresholdCompare(tempValue);   //Compare readout temperature with set themperature threshold value. Fault code is active 'true' if readout temperature value is higher than temperature treshold set by rotary encoder.
  
  
  uint16_t lightValue;                                          
  uint16_t uvValue;                                             
  //uint16_t irValue;                                             
  lightValue = lightSensor.ReadVisible();                           //Light readout, unit in lumen.
  uvValue = lightSensor.ReadUV();                                   //UV readout, unit in lumen.
  //irValue = lightSensor.ReadIR();                                   //IR readout, unit in UN index.

  ledLightEnabled = ledLights.checkLightNeed(uvValue);              //Check if LED lighting is needed and if it is allowed to be turned ON.
  ledLightState = ledLights.startLed();                             //Start LED lighting if it is enabled and update its current state.
  ledLightFault = ledLights.ledLightCheck(uvValue);                 //Check if LED lights functionality. Fault code is active 'true' if readout light value is not increased while LED lighting is turned ON.
  ledLightState = ledLights.stopLed();                              //Stop LED lighting if it is not enabled and update its current state.

  unsigned short waterFlowValue;                                
  waterPumpEnabled = waterPump.checkWaterNeed();                    //Check if water is needed and if water pump is allowed to be turned ON.
  waterLevelFault = waterPump.readWaterLevel();                     //Check water level in water tank.
  //waterPumpState = waterPump.startPump(&waterFlowValue);            //Start water pump if it is enabled, calculate the flow in which water is being pumped and update the water pump's current state.
  waterFlowFault = waterPump.flowCheck(waterFlowValue);             //Check if water flow is above threshold value when water pump is running.                       
  //waterPumpState = waterPump.stopPump();                            //Stop water pump (OFF).

  oledDisplay.printToScreen(STARTUP_IMAGE);
  //oledDisplay.printToScreen(SET_TIME);
  //oledDisplay.printToScreen(READOUT_VALUES);
  //oledDisplay.printToScreen(SERVICE_MODE);
  //oledDisplay.printToScreen(FLOW_FAULT);
  
  //Set current time and toggle water flow fault code status.
  pushButton1 = digitalRead(clockSetButton);                        //Check if SET-button is being pressed.

/*
  //Different functions to be run depending of which screen display mode that is currently active.
  if(startupImageDisplay == true) {
    viewStartupImage();                                             //Initialize the OLED Display and show startup images.
  }
  else if(setTimeDisplay == true) {                                 //Display time set screen only if current time has not been set.
    setClockTime();
    setClockDisplay();
  }
  else if(readoutValuesDisplay == true) {                           //Only display read out values after current time on internal clock, has been set.
    viewReadoutValues();                                            //Print read out values from the greenhouse to display.
  }
  else if(serviceModeDisplay == true) {
    viewServiceMode();                                              //Service mode screen is printed to display.
  }
  else if(flowFaultDisplay == true) {
    waterFlowFault = resolveFlowFault();                            //Water flow fault display mode is printed to display. It contains fault code instruction and possibility to reset fault code.
  }
  
  //Continuesly read out sensor values, calculate values and alert user if any fault code is set.

  moistureMeanValue = calculateMoistureMean(moistureValue1, moistureValue2, moistureValue3, moistureValue4);    //Mean value from all sensor readouts.
  
  tempValue = humiditySensor.readTemperature(false);                                                    //Read temperature value from DHT-sensor. "false" gives the value in °C.
  //humidValue = humiditySensor.readHumidity();                                                           //Read humidity value from DHT-sensor.
  tempValueFault = tempThresholdCompare();
  
  lightRead();                                                                                          //Read light sensor UV value.
  waterLevelFault = waterLevelRead();                                                                                     //Check water level in water tank.
  alarmMessageDisplay();                                                                                //Print alarm messages to display for any faults that is currently active. Warning messages on display will alert user to take action to solve a certain fault.  

  //Greenhouse program start.
  if(greenhouseProgramStart == true) {                  //When set to 'true' automatic water and lighting control of greenhouse is turned on.
    
    //Check readout light value according to a time cycle and turn led lighting ON/OFF based on the readout.
    unsigned long checkLightNeedCurrent;

    checkLightNeedCurrent = millis();                 //Get current time stamp from millis().
    if(checkLightNeedCurrent - checkLightNeedStart >= checkLightNeedPeriod) {  //Check if time period has elapsed.
      checkLightNeed();                               //Time period has elapsed. Enable/Disable start of LED lighting.
      if(ledLightEnabled == true) {
        ledLightStart();                              //Start(ON) LED lighting.
      }
      else {
        ledLightStop();                               //Stop(OFF) LED lighting.
      }                
      checkLightNeedStart = millis();                 //Get current time stamp from millis() to make it loop.   
    }
    
    //Check readout light value after led lighting has been turned on. This will check if led lighting is working. If not it will set an alarm.
    unsigned long checkLightFaultCurrent;
    
    if(ledLightState == true) {
      checkLightFaultCurrent = millis();              //Get current time stamp from millis().
      if(checkLightFaultCurrent - checkLightFaultStart >= checkLightFaultPeriod) { //Check if time period has elapsed.
        ledLightCheck();                              //Time period has elapsed. Check if LED lighting is working.
        checkLightFaultStart = millis();              //Get current time stamp from millis() to make it loop.                                
      }
    }

    //Check readout moisture value according to a time cycle and enable water pump to start.
    unsigned long checkMoistureCurrent;

    checkMoistureCurrent = millis();                  //Get current time stamp from millis().
    if(checkMoistureCurrent - checkMoistureStart >= checkMoisturePeriod) {    //Check if time period has elapsed.                                                             
      checkWaterNeed();                               //Time period has elapsed. Enable/Disable start of water pump.
      //Start water pump.
      if(waterPumpEnabled == true) {
        waterPumpStart();                               //Start water pump (ON).
        waterPumpTimeStart = millis();                //Get current time stamp from millis() to make it loop.
        checkWaterFlowStart = millis();               //Get current time stamp from millis() to make it loop.
      }
      checkMoistureStart = millis();                  //Get current time stamp from millis() to make it loop.                                                                                       
    }
    
    //Stop water pump after it has run for a certain amount of time.
    unsigned long waterPumpTimeCurrent; 

    if(waterPumpState == true) {  
      waterPumpTimeCurrent = millis();                  //Get current time stamp from millis().
      if(waterPumpTimeCurrent - waterPumpTimeStart >= waterPumpTimePeriod) {  //Check if time period has elapsed.
        waterPumpStop();                              //Time period has elapsed. Stop water pump (OFF).
        waterPumpEnabled = false;                     //Disable water pump from running until next time moisture value readout.
      } 
    }
    
    //Check if water is being pumped when water pump is running by checking the water flow sensor. If not it will set an alarm.
    unsigned long checkWaterFlowCurrent;

    if(waterPumpState == true) {
      checkWaterFlowCurrent = millis();                 //Get current time stamp of millis().
      if(checkWaterFlowCurrent - checkWaterFlowStart >= checkWaterFlowPeriod) {     //Check if time period has elapsed.
        waterFlowCheck();                             //Time period has elapsed. Check water flow.
      }
    }
  }  
  */                                                                    
}
