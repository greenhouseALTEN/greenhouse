/*
*************************
* Included header files *
*************************/
#include "Wire.h"
#include "SeeedGrayOLED.h"
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
static Display oledDisplay;                      //OLED display object created from Display class. 

//Internal clock.
static Clock internalClock;                      //Clock object created from ClockTime class.


//Alarm messages to display.
unsigned long alarmTimePrev = 0;        //Used to read relative time 
unsigned long alarmTimePeriod = 2100;   //Variable value specifies in milliseconds, for how long time each warning message will be shown on display before cleared and/or replaced by next warning message.


/*
---------------------
|Greenhouse program.|
--------------------*/


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
  SeeedGrayOled.setTextXY(0, 42);
  SeeedGrayOled.putString("      ");
  SeeedGrayOled.setTextXY(1, 42);
  SeeedGrayOled.putString("      ");
  SeeedGrayOled.setTextXY(1, 37);
  SeeedGrayOled.putString("     ");  
  SeeedGrayOled.setTextXY(2, 42);
  SeeedGrayOled.putString("      ");    
  SeeedGrayOled.setTextXY(3, 39);
  SeeedGrayOled.putString("         ");
  SeeedGrayOled.setTextXY(4, 42);
  SeeedGrayOled.putString("    ");
  SeeedGrayOled.setTextXY(4, 44);               //Clear symbols from previous display mode.
  SeeedGrayOled.putString("    ");
  SeeedGrayOled.setTextXY(5, 42);
  SeeedGrayOled.putString("      ");

  //Printing read out values from the greenhouse to display.
  ************************
  |Moisture sensor value.|
  ************************/
 /* SeeedGrayOled.setTextXY(0, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Moisture: ");        //Print string to display.
  SeeedGrayOled.setTextXY(0, 42);
  SeeedGrayOled.putNumber(moistureMeanValue);   //Print mean moisture value to display.

  //Select which text string to be printed to display depending of soil moisture.
  SeeedGrayOled.setTextXY(0, 46);
  if(moistureDry == true && moistureWet == false) {
    SeeedGrayOled.putString("Lo");             //Print string to display.
  }
  else if(moistureDry == false && moistureWet == false) {
    SeeedGrayOled.putString("OK");             //Print string to display.
  }
  else if(moistureDry == false && moistureWet == true) {
    SeeedGrayOled.putString("Hi");             //Print string to display.
  }  

  /*******************
  |Temp sensor value.|
  ********************/
  /*SeeedGrayOled.setTextXY(1, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Temp: ");            //Print string to display.
  SeeedGrayOled.setTextXY(1, 42);
  SeeedGrayOled.putNumber(tempValue);           //Print temperature value to display.

  /***********************
  |UV-light sensor value.|
  ************************/
  /*SeeedGrayOled.setTextXY(2, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("UV-light: ");        //Print string to display.
  SeeedGrayOled.setTextXY(2, 42);
  SeeedGrayOled.putNumber(uvValue);             //Print UV-light value to display.

  /********************
  |Light sensor value.|
  *********************/
  /*SeeedGrayOled.setTextXY(3, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Light: ");           //Print string to display.
  SeeedGrayOled.setTextXY(3, 42);
  SeeedGrayOled.putNumber(lightValue);          //Print light value in the unit, lux, to display.
  SeeedGrayOled.putString("lm");                 //Print unit of the value.

  /**********************
  |Temp threshold value.|
  ***********************/
  /*SeeedGrayOled.setTextXY(4, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Temp lim: ");        //Print string to display.
  SeeedGrayOled.setTextXY(4, 42);
  SeeedGrayOled.putNumber(tempThresholdValue / 2);    //Print temperature threshold value to display. Value 24 corresponds to 12°C, temp value is doubled to reduce rotary sensitivity and increase knob rotation precision.
  
  /*************************
  |Water flow sensor value.|
  **************************/
  /*SeeedGrayOled.setTextXY(5, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Flow Sens: ");       //Print string to display.
  SeeedGrayOled.setTextXY(5, 42);
  SeeedGrayOled.putNumber(waterFlowValue);      //Print water flow value to display.
  SeeedGrayOled.putString("L/h");               //Print unit of the value.

  //Printing separator line to separate read out values from error/warning messages.
  SeeedGrayOled.setTextXY(6, 0);                //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("----------------");  //Print string to display.
}*/





/*
=======================================================================================================================================================
|| Timer interrupt to read temperature threshold value adjustments (set by rotary encoder) and to make pointer digits flash when in "set clock mode" ||
======================================================================================================================================================= */
/*
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
      characterFlashEnabled = true;
    }
    else {
      characterFlashEnabled = false;
      x = 0;
    }
  }
} 
*/
  
/*
=============================================================
|| Timer interrupt to feed clock function with 1Hz signal. ||
============================================================= */
/*
ISR(TIMER1_COMPA_vect) {  //Timer interrupt with a frequency of 1Hz to feed internal clock with signal to increment its second pointer.
  //Internal clock used to keep track of current time. This function will be run once every second.
  if(clockStartEnabled = true) {      //When set 'true' clock starts ticking.                       
      
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
*/





/* AVAILABLE IN DISPLAY CLASS!!!
===================================================================================================
|| SET CLOCK TIME DISPLAY MODE. Print clock values to OLED display to let user set current time. ||
=================================================================================================== */
/* 
void setClockDisplay() {
  SeeedGrayOled.setTextXY(0, 0);                            //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Set current time");              //Print text to display.
  SeeedGrayOled.setTextXY(1, 0);                            //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Use buttons:");                  
  SeeedGrayOled.setTextXY(2, 0);                            
  SeeedGrayOled.putString("SET = inc. p.val");              
  SeeedGrayOled.setTextXY(3, 0);                            
  SeeedGrayOled.putString("MODE = h or min");               
  SeeedGrayOled.setTextXY(5, 20);                           
  SeeedGrayOled.putString("HH");                            
  SeeedGrayOled.setTextXY(5, 23);                           
  SeeedGrayOled.putString("MM");                            
  SeeedGrayOled.setTextXY(5, 26);                           
  SeeedGrayOled.putString("SS");                            

  //Flashing individual clock time pointers to display which clock parameter that is currently set.
  //Hour pointer2
  if(characterFlashEnabled == true && hour2InputMode == true) {
    SeeedGrayOled.setTextXY(6, 20);                           
    SeeedGrayOled.putString(" ");                             //Clear display where 10-digit hour pointer value is located.    
  }
  else {
    SeeedGrayOled.setTextXY(6, 20);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedGrayOled.putNumber(hourPointer2);                    //Print 10-digit hour pointer value to display.
  }
  
  //Hour pointer1.
  if(characterFlashEnabled == true && hour1InputMode == true) {
    SeeedGrayOled.setTextXY(6, 21);                           
    SeeedGrayOled.putString(" ");                             //Clear display where 1-digit hour pointer value is located.

  }
  else {
    SeeedGrayOled.setTextXY(6, 21);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
    SeeedGrayOled.putNumber(hourPointer1);                    //Print 1-digit hour pointer value to display.
  }

  //Pointer separator symbol.
  SeeedGrayOled.setTextXY(6, 22);                             //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString(":");                               //Print symbol to display.

  //Minute pointer2.
  if(characterFlashEnabled == true && minute2InputMode == true) {
    SeeedGrayOled.setTextXY(6, 23);                           
    SeeedGrayOled.putString(" ");    
  }                  
  else {
    SeeedGrayOled.setTextXY(6, 23);                           
    SeeedGrayOled.putNumber(minutePointer2);
  }

  //Minute pointer1.
  if(characterFlashEnabled == true && minute1InputMode == true) {
    SeeedGrayOled.setTextXY(6, 24);                           
    SeeedGrayOled.putString(" ");                
  }
  else {
    SeeedGrayOled.setTextXY(6, 24);                           
    SeeedGrayOled.putNumber(minutePointer1);   
  }

  //Pointer separator symbol.
  SeeedGrayOled.setTextXY(6, 25);                           
  SeeedGrayOled.putString(":");

  //Second pointers.
  SeeedGrayOled.setTextXY(6, 26);                           
  SeeedGrayOled.putNumber(secondPointer2);                   //Print second digit of second pointer value to display.
  SeeedGrayOled.setTextXY(6, 27);                           
  SeeedGrayOled.putNumber(secondPointer1);                   //Print first digit of second pointer value to display.
}
*/

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
      SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
      SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
      if(waterFlowFault == true) {                        //If fault variable is set to 'true', fault message is printed to display.
        SeeedGrayOled.setTextXY(7, 0);                        
        SeeedGrayOled.putString("No water flow");             //Print fault message to display.
      }

      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(7, 0);                        //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");          //Clear row to enable other warnings to be printed to display.
      }
    }

    /**********************
    |Low water tank level.|
    ***********************/
    if(alarmTimePeriod < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 2) {
      if(waterLevelFault == true) {                         //If fault variable is set to 'true', fault message is printed to display.
        SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedGrayOled.setTextXY(7, 0);                          
        SeeedGrayOled.putString("Low water level");             //Print fault message to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.      
      }
    }

    /*******************
    |Temperature fault.|
    ********************/
    if(alarmTimePeriod * 2 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 3) { 
      if(tempValueFault == true) {                          //If fault variable is set to 'true', fault message is printed to display.
        SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedGrayOled.setTextXY(7, 0);                          
        SeeedGrayOled.putString("High temperature");            //Print fault message to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.      
      }
    }

    /********************
    |LED lighting fault.|
    *********************/
    if(alarmTimePeriod * 3 < alarmTimeDiff && alarmTimeDiff <= alarmTimePeriod * 4) { 
      if(ledLightFault == true) {                           //If fault variable is set to 'true', fault message is printed to display.
        SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to which row that will be cleared.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
        SeeedGrayOled.setTextXY(7, 0);                          
        SeeedGrayOled.putString("LED not working");             //If measured water flow is below a certain value without the water level sensor indicating the water tank is empty, there is a problem with the water tank hose. "Check water hose!" is printed to display.
      }
      else {  //If this alarm not active, clear the warning message row.
        SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
        SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.      
      }
    }
  
    if(alarmTimeDiff > alarmTimePeriod * 4) {
      SeeedGrayOled.setTextXY(7, 0);                          //Set cordinates to the warning message will be printed.
      SeeedGrayOled.putString("                ");            //Clear row to enable other warnings to be printed to display.
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
  SeeedGrayOled.setTextXY(0, 39);
  SeeedGrayOled.putString(" ");
  SeeedGrayOled.setTextXY(1, 25);  
  SeeedGrayOled.putString("     ");  
  SeeedGrayOled.setTextXY(2, 19);
  SeeedGrayOled.putString("   ");
  SeeedGrayOled.setTextXY(2, 24);                            
  SeeedGrayOled.putString(" ");  
  SeeedGrayOled.setTextXY(2, 28);
  SeeedGrayOled.putString("   ");     
  SeeedGrayOled.setTextXY(3, 19);
  SeeedGrayOled.putString("   ");
  SeeedGrayOled.setTextXY(3, 28);
  SeeedGrayOled.putString("   ");
  SeeedGrayOled.setTextXY(4, 27);
  SeeedGrayOled.putString(" ");
  SeeedGrayOled.setTextXY(5, 26);
  SeeedGrayOled.putString("  ");
  SeeedGrayOled.setTextXY(5, 45);
  SeeedGrayOled.putString("   ");
  SeeedGrayOled.setTextXY(6, 42);
  SeeedGrayOled.putString("  ");
  SeeedGrayOled.setTextXY(6, 45);
  SeeedGrayOled.putString("   ");
  SeeedGrayOled.setTextXY(7, 45);
  SeeedGrayOled.putString("  ");
  
  //Display clock.
  SeeedGrayOled.setTextXY(0, 0);                            //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Clock: ");                       //Print string to display.
  //Hour pointer.
  SeeedGrayOled.setTextXY(0, 40);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(hourPointer2);                    //Print 10-digit hour pointer value to display.
  SeeedGrayOled.setTextXY(0, 41);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(hourPointer1);                    //Print 1-digit hour pointer value to display.
  
  SeeedGrayOled.setTextXY(0, 42);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString(":");                             //Print separator symbol, between hour and minute digits, todisplay.

  //Minute pointer.
  SeeedGrayOled.setTextXY(0, 43);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(minutePointer2);                  //Print 10-digit hour pointer value to display.
  SeeedGrayOled.setTextXY(0, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(minutePointer1);                  //Print 1-digit hour pointer value to display.

  SeeedGrayOled.setTextXY(0, 45);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString(":");                             //Print separator symbol, between hour and minute digits, todisplay.

  //Second pointer.
  SeeedGrayOled.setTextXY(0, 46);                           
  SeeedGrayOled.putNumber(secondPointer2);                  //Print second digit of second pointer value to display.
  SeeedGrayOled.setTextXY(0, 47);                           
  SeeedGrayOled.putNumber(secondPointer1);                  //Print first digit of second pointer value to display.

  //Display moisture sensor values.
  SeeedGrayOled.setTextXY(1, 0);                            //Set cordinates to where it will print text. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putString("Moisture:");                     //Print text to display.
  SeeedGrayOled.setTextXY(2, 0);                            
  SeeedGrayOled.putString("S1[");                           //Print text to display.
  SeeedGrayOled.setTextXY(2, 19);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue1);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(2, 22);                            
  SeeedGrayOled.putString("],");                     

  SeeedGrayOled.setTextXY(2, 25);                            
  SeeedGrayOled.putString("S2[");                           //Print text to display.
  SeeedGrayOled.setTextXY(2, 28);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue2);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(2, 31);                            
  SeeedGrayOled.putString("]");
  
  SeeedGrayOled.setTextXY(3, 0);                            
  SeeedGrayOled.putString("S3[");                           //Print text to display.
  SeeedGrayOled.setTextXY(3, 19);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue3);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(3, 22);                            
  SeeedGrayOled.putString("],");                     

  SeeedGrayOled.setTextXY(3, 25);                            
  SeeedGrayOled.putString("S4[");                           //Print text to display.
  SeeedGrayOled.setTextXY(3, 28);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(moistureValue4);                  //Print moisture sensor1 value.
  SeeedGrayOled.setTextXY(3, 31);                            
  SeeedGrayOled.putString("]");

  //Alarm messsage status.
  SeeedGrayOled.setTextXY(4, 0);                            
  SeeedGrayOled.putString("tempValue: ");                   //Print text to display.
  SeeedGrayOled.setTextXY(4, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(tempValueFault);                  //Print moisture sensor1 value.

  SeeedGrayOled.setTextXY(5, 0);                            
  SeeedGrayOled.putString("ledLight: ");                    //Print text to display.
  SeeedGrayOled.setTextXY(5, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(ledLightFault);                   //Print moisture sensor1 value.

  SeeedGrayOled.setTextXY(6, 0);                            
  SeeedGrayOled.putString("waterFlow: ");                   //Print text to display.
  SeeedGrayOled.setTextXY(6, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(waterFlowFault);                  //Print moisture sensor1 value.

  SeeedGrayOled.setTextXY(7, 0);                            
  SeeedGrayOled.putString("waterLevel: ");                  //Print text to display.
  SeeedGrayOled.setTextXY(7, 44);                           //Set cordinates to where any text print will be printed to display. X = row (0-7), Y = column (0-127).
  SeeedGrayOled.putNumber(waterLevelFault);                 //Print moisture sensor1 value.
}
*/

/*
=========================================================================
|| FLOW FAULT DISPLAY MODE. Print service mode screen to OLED display. ||
========================================================================= */
/*
bool resolveFlowFault() {    
  bool fault;
  
  //Clear redundant symbols from previous screen mode.
  SeeedGrayOled.setTextXY(0, 28);
  SeeedGrayOled.putString("    ");
  SeeedGrayOled.setTextXY(2, 45);
  SeeedGrayOled.putString("   ");
  SeeedGrayOled.setTextXY(5, 45);
  SeeedGrayOled.putString("   ");
  
  //Print fault code instruction to display. To let user resolve fault.
  SeeedGrayOled.setTextXY(0, 0);
  SeeedGrayOled.putString("waterFlow: ");
  SeeedGrayOled.setTextXY(0, 27);
  SeeedGrayOled.putNumber(waterFlowFault);
  
  SeeedGrayOled.setTextXY(1, 0);
  SeeedGrayOled.putString("----------------");
  SeeedGrayOled.setTextXY(2, 0);
  SeeedGrayOled.putString("Chk hardware: ");
  SeeedGrayOled.setTextXY(3, 0);
  SeeedGrayOled.putString("*is hose empty?");
  SeeedGrayOled.setTextXY(4, 0);
  SeeedGrayOled.putString("*vacuum in tank?");
  SeeedGrayOled.setTextXY(5, 0);
  SeeedGrayOled.putString("*Done? Press:");
  SeeedGrayOled.setTextXY(6, 0);
  SeeedGrayOled.putString("SET = clear/set");
  SeeedGrayOled.setTextXY(7, 0);
  SeeedGrayOled.putString("MODE = confirm");

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
*/

/*
===========================================================================================================================================================
|| Timer interrupt triggered with frequency of 10 Hz used as second ticker for internal clock and to flash clock pointer values when in "set time" mode. ||
=========================================================================================================================================================== */
ISR(RTC_CNT_vect) {
  //Timer interrupt triggered with a frequency of 10 Hz.
  /***************************************************
  |Internal clock used to keep track of current time.|
  ****************************************************/
  if(clockStartEnabled == true) {           //Check if internal clock is enabled to run.
    divider10++;                            //Increase divider variable.                   
      
    if(divider10 == 10) {                   //This part of the function will run once every second and therefore will provide a 1Hz pulse to feed the second pointer."
      divider10 = 0;                        //Clear divider variable.
      secondPointer1++;                     //Increase second pointer every time this function runs.
      
      //Second pointer.
      if(secondPointer1 == 10) {            //If 1-digit second pointer reaches a value of 10 (elapsed time is 10 seconds).
        secondPointer2++;                   //Increase 10-digit second pointer.
        secondPointer1 = 0;                 //Clear 1-digit pointer.
      }
      if(secondPointer2 == 6) {             //If 10-digit pointer reaches a value of 6 (elapsed time is 60 seconds).
        minutePointer1++;                   //Increase minute pointer.
        secondPointer2 = 0;                 //Clear 10-digit second pointer.
      }
      //Minute pointer.
      if(minutePointer1 == 10) {            //If 1-digit minute pointer reaches a value of 10 (elapsed time is 10 minutes).
        minutePointer2++;                   //Increase 10-digit minute pointer.
        minutePointer1 = 0;                 //Clear 1-digit minute pointer.
      }
      if(minutePointer2 == 6) {             //If 10-digit minute pointer reaches a value of 6 (elapsed time is 60 minutes).
        hourPointer1++;                     //Increase 1-digit hour pointer.
        minutePointer2 = 0;                 //Clear 10-digit minute pointer.
      }
      //Hour pointer.
      if(hourPointer1 == 10) {              //If 1-digit hour pointer reaches a value of 10 (elapsed time is 10 hours).
        hourPointer2++;                     //Increase 10-digit hour pointer.
        hourPointer1 = 0;                   //Clear 1-digit hour pointer.
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
  RTC.INTFLAGS = 0x3;                       //Clearing OVF and CMP interrupt flags to enable new interrupt to take place according the preset time period.
  
}

/*
*******************************
* Arduino program setup code. *
*******************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //Initiate the OLED display.
  Wire.begin();
  SeeedGrayOled.init(SH1107G);          //Initialize OLED-Display.
  SeeedGrayOled.clearDisplay();         //Clear display and set start position to top left corner.
  SeeedGrayOled.setNormalDisplay();     //Set display to normal mode (screen is dark only text is enlightened).
  SeeedGrayOled.setGrayLevel(100);      //Adjust gray scale value 0-255. 
  SeeedGrayOled.setContrastLevel(100);  //Adjust contrast value 0-255.

  pinMode(SENSOR_PORT1, INPUT);
  pinMode(SENSOR_PORT2, INPUT);
  pinMode(SENSOR_PORT3, INPUT);
  pinMode(SENSOR_PORT4, INPUT);
  
  pinMode(flowSensor, INPUT);
  attachInterrupt(1, Watering::flowCount, RISING);          //Initialize interrupt to enable water flow sensor to calculate water flow pumped by water pump.

  attachInterrupt(0, Display::toggleDisplayMode, RISING);   //Initialize interrupt to toggle set modes when in clock set mode or toggling screen display mode when greenhouse program is running. Interrupt is triggered by clockModeButton being pressed.
  
  pinMode(pumpRelay, OUTPUT);;
  
  pinMode(waterLevelSwitch, INPUT);
  
  pinMode(lightRelay, OUTPUT);
  
  pinMode(rotaryEncoderOutpA, INPUT);
  pinMode(rotaryEncoderOutpB, INPUT);
  aLastState = digitalRead(rotaryEncoderOutpA);      //Read initial position value.

  pinMode(SET_BUTTON, INPUT);
  pinMode(MODE_BUTTON, INPUT);
  
  humiditySensor.begin();                 //Initializing humidity sensor.
  
  lightSensor.Begin();                    //Initializing light sensor.

  cli();                                              //Stop any external interrups.

  //RTC setup:
  while(RTC.STATUS != 0) {
    //Wait until the CTRLABUSY bit in register is cleared before writing to CTRLA register.
    Serial.println("waiting for 1");                          
  } 
  RTC.CLKSEL = 0x00;                                  //32.768 kHz signal from OSCULP32K selected.
  RTC.PERL = 0x0A;                                    //Lower part of 16,384 value in PER-register (PERL) to be used as overflow value to reset the RTC counter.
  RTC.PERH = 0x10;                                    //Upper part of 16,384 value in PER-register (PERH) to be used as overflow value to reset the RTC counter.
  RTC.INTCTRL = (RTC.INTCTRL & 0b11111100) | 0b01;    //Enable interrupt-on-counter overflow by setting OVF-bit in INCTRL register.
  while(RTC.STATUS != 0) {
    //Wait until the CTRLABUSY bit in register is cleared before writing to CTRLA register.   
    Serial.println("waiting for 2");               
  }                                                      
  RTC.CTRLA = 0x05;                                   //No using prescaler set, CORREN enabled (0b100),  RTCEN bit set to 1 (0b1).
                                         
  while(RTC.STATUS != 0) {
    //Wait until the CTRLABUSY bit in register is cleared before writing to CTRLA register. 
    Serial.println("waiting for 3");                        
  }  
  Serial.println("RTC config complete");

  sei();                                              //Allow external interrupt again.   
                                         
  /*
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
  */
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

  //Set current time and clear/set water flow fault code.
  setButton = digitalRead(SET_BUTTON);                              //Check if SET-button is being pressed.

  oledDisplay.printToScreen();                                      //Print current display state to display. Display state variable change in the interrupt function activated by pressing MODE-button. 


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
