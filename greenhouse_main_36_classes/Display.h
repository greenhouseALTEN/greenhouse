#ifndef Display_H_
#define Display_H_
#include "Arduino.h"
#include "Miscellaneous.h"

class Display {
public:
  Display(){}                                   //Empty constructor.
  void printToScreen(displayMode x);
private:
  void viewStartupImage();
  void stringToDisplay(unsigned short x, unsigned short y, char text[]);
  void numberToDisplay(unsigned short x, unsigned short y, int variable);
  void blankToDisplay(unsigned short x, unsigned short y, int numOfBlank);
};

#endif  /* Display_H_ */
