#include "Clock.h"
#include "Miscellaneous.h"

/*
===============================================================
|| Set current time by using SET- and MODE-button as inputs. ||
=============================================================== */
void Clock::setTime() {
  switch(clockInputState) {
    case HOUR2:                                                 //Set hour pointer1.
      if(setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                            //Delay to avoid contact bounce.
        hourPointer2++;                                         //Increase hour pointer every time button is pressed.
          if(hourPointer2 == 3) {                               //If 10-digit hour pointer reaches 3, clear digit.
            hourPointer2 = 0;
          }
      }
      break;
    case HOUR1:                                                 //Set hour pointer2.
      if(setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                            //Delay to avoid contact bounce.
        hourPointer1++;                                         //Increase hour pointer every time button is pressed.

        if(hourPointer2 == 2) {                                 //If hour pointer2 is equal to 2, hour pointer 1 is only allowed to reach a maximum value of 4.
          if(hourPointer1 == 5) {
            hourPointer1 = 0;
          }
        }
        else {
          if(hourPointer1 == 10) {                              //If 1-digit hour pointer reaches 10, clear digit.
            hourPointer1 = 0;
          }
        }
      }
      break;
    case MINUTE2:                                               //Set ninute pointer2.
      if(setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                                                 
        minutePointer2++;
        if(minutePointer2 == 6) {                               //If 10-digit minute pointer reaches a value of 6, clear 10-digit minute pointer.
          minutePointer2 = 0;    
        }
      }
      break;
    case MINUTE1:                                               //Set minute pointer1.
      if(setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                                                 
        minutePointer1++;
        if(minutePointer1 == 10) {                              //If 10-digit minute pointer reaches a value of 10, clear 1-digit minute pointer.
          minutePointer1 = 0;    
        }
      } 
   }

  //Replace clock time represenation. When current clock time is 24 hours is replaced with 00.
  if(clockStartEnabled == true) {
    if(hourPointer2 == 2 && hourPointer1 == 4) {                //If 10-digit hour pointer reaches a value of 2 and 1-digit hour pointer reaches a value of 4 (elapsed time is 24 hours).
      hourPointer2 = 0;                                         //Clear both hour pointer values.
      hourPointer1 = 0;
    }
  }
}

/*
=================
|| Reset clock ||
================= */
void Clock::resetTime() {
  //Stop clock and reset all clock pointers.
  clockStartEnabled = false;              //Stop clock from ticking.
  hourPointer2 = 0;                       //Reset all clock pointers individualy.
  hourPointer1 = 0;
  minutePointer2 = 0;
  minutePointer1 = 0; 
  secondPointer2 = 0;                           
  secondPointer1 = 0; 
  Serial.println("resetTime");                                           
}
