#ifndef Display_H_
#define Display_H_
#include "Arduino.h"
#include "Miscellaneous.h"

class Display {
public:
  static Display *getInstance();                  //Return the pointer object of the class. 
  void printToScreen();
  static void toggleDisplayMode();
 
private:
  Display() {}                                    //Empty constructor.
  static Display *oledDisplay;                    //Declaring pointer object of the class.
  void viewStartupImage();
  void viewSetClock();
  void viewReadoutValues();
  void stringToDisplay(unsigned char x, unsigned char y, char* text);
  void numberToDisplay(unsigned char x, unsigned char y, int variable);
  void blankToDisplay(unsigned char x, unsigned char y, int numOfBlanks);
  void flashNumberDisplay(unsigned short x, unsigned short y, unsigned short numOfDigits);
  String soilStatDisplay(bool moistureDry, bool moistureWet); 
};

#endif  /* Display_H_ */
