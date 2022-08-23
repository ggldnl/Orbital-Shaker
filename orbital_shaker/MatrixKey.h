/*
MatrixKeys.h - 4 x 4 keyboard matrix scanning library.


In a parallel process, the scanState () method should be called regularly. 
As a result, the signs of flagPress [4] [4] and flagClick [4] [4] arrays are formed:
- when the button is pressed flagPress = true;
- when the button is free flagPress = false;
- at the moment of pressing the button flagClick = true button.

The constructor creates an object with the specified parameters.
- verticalPin1, verticalPin2, verticalPin3, verticalPin4 - pin numbers for connecting vertical lines of the matrix 1, 2, 3, 4;
- horizontalPin1, horizontalPin2, horizontalPin3, horizontalPin4 - pin numbers for connecting horizontal matrix lines 1, 2, 3, 4;
- numAckn - the number of confirmations of the status of contacts.

Separate matrix pins can be disabled by setting the numbers for them in the constructor to 255.
 
An example of creating an object:
// create an object matrix of keys
// connect vertical lines to pins 9, 10, 11, 12
// connect horizontal lines to pins 4, 5, 6, 7
// number of confirmations of the status of contacts = 6
MatrixKeys keys (9, 10, 11, 12, 4, 5, 6, 7, 6);
   
 Library developed by Kalinin Edward
 http://mypractic.com/ 
 Lesson 18.
*/

// checking that the library is not yet included
#ifndef MatrixKeys_h // if the MatrixKeys library is not included
#define MatrixKeys_h // then include it

#include "Arduino.h"

// description of the class of scanning matrix of buttons
class MatrixKeys  {
  public:
    bool flagPress[4][4]; // signs - the button is pressed
    bool flagClick[4][4]; // signs - the button was pressed (click)
    void  scanState();    // button state check method

    // returns true if the button exists (!= 255)
    bool isValid(int row, int col);

    // disables the keypad
    void disable (void);

    // enables the keypad
    void enable (void);
    
    // constructor
    MatrixKeys(byte verticalPin1, byte verticalPin2, byte verticalPin3, byte verticalPin4, 
               byte horizontalPin1, byte horizontalPin2, byte horizontalPin3, byte horizontalPin4, 
               byte numAckn);    
  private:
    byte  _verticalPins[4];   // vertical lines pins
    byte  _horizontalPins[4]; // horizontal lines pins
    byte  _buttonCount[4][4]; // counters confirm the status of buttons
    byte _numAckn;          // number of confirmations of the state of the buttons
    byte  _scanVertLine;    // number of the scanned vertical line
    byte  _scanHorizLine;    // number of scanned horizontal line
    bool _enabled;
} ;

#endif
