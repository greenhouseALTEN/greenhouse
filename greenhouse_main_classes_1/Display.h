#ifndef Display_H_
#define Display_H_
#include "Arduino.h"
#include "Miscellaneous.h"

class Display {
public:
  Display(){}                                   //Empty constructor.
  void printToScreen();
  static void toggleDisplayMode();
private:
  void viewStartupImage();
  void viewSetClock();
  void stringToDisplay(unsigned char x, unsigned char y, char text[]);
  void numberToDisplay(unsigned char x, unsigned char y, int variable);
  void blankToDisplay(unsigned char x, unsigned char y, int numOfBlanks);
  void flashNumberDisplay(unsigned short x, unsigned short y, unsigned short numOfDigits);
};

#endif  /* Display_H_ */
