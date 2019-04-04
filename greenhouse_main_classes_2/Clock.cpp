#include "Clock.h"
#include "Miscellaneous.h"

/*
===============================================================
|| Set current time by using SET- and MODE-button as inputs. ||
=============================================================== */
void Clock::setTime() {
  switch(Config::State().clockInputState_x) {
    case 0/*HOUR2*/:                                                 //Set hour cursor1.
      if(Config::State().setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                            //Delay to avoid contact bounce.
        Serial.println("hello");
        Config::State().hourCursor2++;                                         //Increase hour pointer every time button is pressed.
          if(Config::State().hourCursor2 == 3) {                               //If 10-digit hour cursor reaches 3, clear digit.
            Config::State().hourCursor2 = 0;
          }
      }
      break;
    case 1/*HOUR1*/:                                                 //Set hour pointer2.
      if(Config::State().setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                            //Delay to avoid contact bounce.
        Serial.println("hello1");
        Config::State().hourCursor1++;                                         //Increase hour pointer every time button is pressed.

        if(Config::State().hourCursor2 == 2) {                                 //If hour pointer2 is equal to 2, hour pointer 1 is only allowed to reach a maximum value of 4.
          if(Config::State().hourCursor1 == 5) {
            Config::State().hourCursor1 = 0;
          }
        }
        else {
          if(Config::State().hourCursor1 == 10) {                              //If 1-digit hour pointer reaches 10, clear digit.
            Config::State().hourCursor1 = 0;
          }
        }
      }
      break;
    case 2 /*MINUTE2*/:                                               //Set ninute pointer2.
      if(Config::State().setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                                                 
        Config::State().minuteCursor2++;
        if(Config::State().minuteCursor2 == 6) {                               //If 10-digit minute pointer reaches a value of 6, clear 10-digit minute pointer.
          Config::State().minuteCursor2 = 0;    
        }
      }
      break;
    case 3 /*MINUTE1*/:                                               //Set minute pointer1.
      if(Config::State().setButton == true) {
        delay(DEBOUNCE_TIME_PERIOD);                                                 
        Config::State().minuteCursor1++;
        if(Config::State().minuteCursor1 == 10) {                              //If 10-digit minute pointer reaches a value of 10, clear 1-digit minute pointer.
          Config::State().minuteCursor1 = 0;    
        }
      }
      Serial.println("setTime");
   }

  //Replace clock time represenation. When current clock time is 24 hours is replaced with 00.
  if(Config::State().clockStartEnabled == true) {
    if(Config::State().hourCursor2 == 2 && Config::State().hourCursor1 == 4) {                //If 10-digit hour pointer reaches a value of 2 and 1-digit hour pointer reaches a value of 4 (elapsed time is 24 hours).
      Config::State().hourCursor2 = 0;                                         //Clear both hour pointer values.
      Config::State().hourCursor1 = 0;
    }
  }
}

/*
=================
|| Reset clock ||
================= */
void Clock::resetTime() {
  //Stop clock and reset all clock pointers.
  Config::State().clockStartEnabled = false;  //Stop clock from ticking.
  Config::State().hourCursor2 = 0;                            //Reset all clock cursors individualy.
  Config::State().hourCursor1 = 0;
  Config::State().minuteCursor2 = 0;
  Config::State().minuteCursor1 = 0; 
  Config::State().secondCursor2 = 0;                           
  Config::State().secondCursor1 = 0; 
  Serial.println("resetTime"); 
}
