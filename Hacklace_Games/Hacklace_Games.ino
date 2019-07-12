/*
 * Hacklace_Main.ino
 *
 */ 

/**********************************************************************************

Description:    Hacklace2 Main Program

                Note: The Hacklace library uses Timer1.
					
Author:		Frank Andre
Copyright 2013:	Frank Andre
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
#include "Hacklace_AppEngine.h"
#include "HL_AppRegistry.h"


// the setup routine runs once when you press reset:
void setup() {
  HL.initialize();
  
  // only for debugging purposes: 
  // ALWAYS restore default EEPROM content after a reset
  //HL.copyToEeprom(ee_default, sizeof(ee_default));
}


// the loop routine runs over and over again forever:
void loop() {

  if (HL.sysTimerHasElapsed()) {    // system timer elapses every 10 ms
    HL.run();
  }

  HL.goToSleep();      // save battery power
}
