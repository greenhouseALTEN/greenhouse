#ifndef Display_H_
#define Display_H_
#include "Arduino.h"
#include "Miscellaneous.h"

class Display {
public:
  Display() {}                                    //Empty constructor.
  void printToScreen();
  static void toggleDisplayMode();
 
private:
  static Clock internalClock;                     //Create a temporary internal clock object from Clock class to be able to access functions within its class.
  static Watering waterPump;                      //Create a temporary water pump object created from Water class to be able to access functions within its class.

  void viewStartupImage();
  void viewSetClock();
  void stringToDisplay(unsigned char x, unsigned char y, char* text);
  void numberToDisplay(unsigned char x, unsigned char y, int variable);
  void blankToDisplay(unsigned char x, unsigned char y, int numOfBlanks);
  void flashNumberDisplay(unsigned short x, unsigned short y, unsigned short numOfDigits);
};

#endif  /* Display_H_ */
