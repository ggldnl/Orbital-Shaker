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

#include "MatrixKey.h"
#include "Arduino.h"

//---------------------------- constructor
MatrixKeys::MatrixKeys(byte verticalPin1, byte verticalPin2, byte verticalPin3, byte verticalPin4, 
                       byte horizontalPin1, byte horizontalPin2, byte horizontalPin3, byte horizontalPin4, 
                       byte numAckn)  {

  // load arrays of pins
  _verticalPins[0]= verticalPin1; _verticalPins[1]= verticalPin2; _verticalPins[2]= verticalPin3; _verticalPins[3]= verticalPin4; 
  _horizontalPins[0]= horizontalPin1; _horizontalPins[1]= horizontalPin2; _horizontalPins[2]= horizontalPin3; _horizontalPins[3]= horizontalPin4; 

  // overload remaining arguments
  _numAckn= numAckn; 
  
  // initialize matrix pins
  byte i= 0;
  while (i < 4) {

    if ( _verticalPins[i] != 255 ) {    // if pin is not disabled    
      pinMode(_verticalPins[i], OUTPUT);
      digitalWrite(_verticalPins[i], LOW);
    }  

    if ( _horizontalPins[i] != 255 )     // if pin is not disabled    
      pinMode(_horizontalPins[i], INPUT);
    
    i++;        
  }


    _scanVertLine= 0;   // number of the scanned vertical line
    _scanHorizLine= 0;  // number of scanned horizontal line

  // reset all signs
    i= 0; while (i < 3) {
            flagPress[i][0]= false; flagPress[i][1]= false; flagPress[i][2]= false; flagPress[i][3]= false; 
            flagClick[i][0]= false; flagClick[i][1]= false; flagClick[i][2]= false; flagClick[i][3]= false;           
            _buttonCount[i][0]= 0; _buttonCount[i][1]= 0; _buttonCount[i][2]= 0; _buttonCount[i][3]= 0; 
            i++;
          }                           

    _enabled = true;
  }
  
//-------------------------------- button state check method
// when the button is pressed flagPress = true 
// when the button is free flagPress = false
// when you click the button flagClick = true
void  MatrixKeys::scanState() {

  if (_enabled) {
  
    if ( _verticalPins[_scanVertLine] != 255 ) {    // if pin is not disabled    
  
    // polling buttons of horizontal lines
    _scanHorizLine= 0;  while ( _scanHorizLine < 4 ) {
      
   if ( flagPress[_scanVertLine][_scanHorizLine] == digitalRead(_horizontalPins[_scanHorizLine]) ) 
      //  button state remains the same 
      _buttonCount[_scanVertLine][_scanHorizLine]= 0;  // reset confirmation count
  
    else  {
       // button state changed
       _buttonCount[_scanVertLine][_scanHorizLine]++;   // +1 to the confirmation counter
  
      if ( _buttonCount[_scanVertLine][_scanHorizLine] >= _numAckn ) {
        // state of the button has become stable
        flagPress[_scanVertLine][_scanHorizLine]= ! flagPress[_scanVertLine][_scanHorizLine]; //  inversion of the status indicator
       _buttonCount[_scanVertLine][_scanHorizLine]= 0;  // reset confirmation count
  
        if ( flagPress[_scanVertLine][_scanHorizLine] == true )
          flagClick[_scanVertLine][_scanHorizLine]= true; // button click sign 
       }       
    }
    _scanHorizLine++; }
    
    }
  
    // setting the next vertical line
      digitalWrite(_verticalPins[_scanVertLine], LOW);
      _scanVertLine++; if (_scanVertLine >3) _scanVertLine= 0;
      digitalWrite(_verticalPins[_scanVertLine], HIGH);         
  }
}

bool MatrixKeys::isValid (int row, int col) {
  return _verticalPins[col] != 255 && _horizontalPins[row] != 255;
}

void MatrixKeys::disable (void) {
  _enabled = false;
}

void MatrixKeys::enable (void) {
  _enabled = true;
}
