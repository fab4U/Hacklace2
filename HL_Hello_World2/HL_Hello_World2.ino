/*
 * HL_Hello_World2.ino
 *
 */ 

/**********************************************************************************

Description:    A slightly more complex "Hello World" program for the Hacklace2
				based on the Hacklace library.

                Note: The Hacklace library uses Timer1.
					
Author:		Frank Andre
Copyright 2015:	Frank Andre
License:	see "license.md"
Disclaimer:	This software is provided by the copyright holder "as is" and any 
		express or implied warranties, including, but not limited to, the 
		implied warranties of merchantability and fitness for a particular 
		purpose are disclaimed. In no event shall the copyright owner or 
		contributors be liable for any direct, indirect, incidental, 
		special, exemplary, or consequential damages (including, but not 
		limited to, procurement of substitute goods or services; loss of 
		use, data, or profits; or business interruption) however caused 
		and on any theory of liability, whether in contract, strict 
		liability, or tort (including negligence or otherwise) arising 
		in any way out of the use of this software, even if advised of 
		the possibility of such damage.
					
**********************************************************************************/
 

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "Hacklace.h"

// global variables
Hacklace hacklace;		// instantiate a Hacklace object
int      toggle = 0;

// the setup routine runs once when you press reset:
void setup() {
  hacklace.initialize();
  hacklace.setScrollSpeed(8, 6);        // scroll speed and duration of pause between two passes
  hacklace.setScrollMode(FORWARD, 1);   // direction = FORWARD, scroll by 1 column at a time
  hacklace.clearDisplay();

  // usage of special character codes
  // here: degree character (= $B0)
  hacklace.printString_P(PSTR("  temperature=27"));
  hacklace.printChar(0xB0);             // print a single degree character
  hacklace.printChar(SPC1);             // print a single column space
  hacklace.printChar('C');
  hacklace.printString_P(PSTR("  position=N12\xB0 34.567  "));   // \xB0 encodes the degree character
}


// the loop routine runs over and over again forever:
void loop() {
  if (hacklace.sysTimerHasElapsed()) {    // system timer elapses every 10 ms
    hacklace.run();          // do the scrolling
  }
  
  if (hacklace.scrollSync()) {          // wait until end of scrolling occurs
    toggle ^= 1;                        // invert toggle
    if (toggle == 1) {
      hacklace.setScrollSpeed(8, 6);
      hacklace.setScrollMode(FORWARD, 1);   // direction = FORWARD, scroll by 1 column at a time
      hacklace.clearDisplay();
      hacklace.printString_P(PSTR("  an animation...  "));
    }
    else {
      hacklace.setScrollMode(FORWARD, 8);  // direction = FORWARD, scroll by 8 columns (= 1 frame) at a time
      hacklace.setScrollSpeed(5, 0);
      hacklace.clearDisplay();
      hacklace.printChar(128+15);          // print animation 15 (dancer)      
    }
  }
  
  hacklace.goToSleep();      // save battery power
}
