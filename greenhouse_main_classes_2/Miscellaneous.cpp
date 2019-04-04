#include "Miscellaneous.h"


Config::Config()
{
    //Internal clock.
    this->setButton = false;                        //Button to increase clock cursor digit value and toggle flow fault code.
    this->flash = false;                            //Toggle variable for flashing clock digit cursor.
    this->clockStartEnabled = false;                //Start/Stop clock internal clock.
    this->hourCursor2 = 0;                          //10-digit of hour cursor.
    this->hourCursor1 = 0;                          //1-digit of hour cursor.
    this->minuteCursor2 = 0;                        //10-digit of minute cursor.
    this->minuteCursor1 = 0;                        //1-digit of minute cursor.
    this->secondCursor2 = 0; 
    this->secondCursor1 = 0; 
    this->currentClockTime = 0;                     //Internal clock time stored as one intiger instead of as separate intigers for each digit.
    this->clockInputState_x = 0;                    //Temp value because typedef enum did not work in Config class.
    //this->
    //this->
}
