/*
 * HL_AnimationApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Animation App

					Display a text or animation from EEPROM. 
					Button 2 increases display brightness.
					
Author:				Frank Andre
Copyright 2013:		Frank Andre
License:			see "license.md"
Disclaimer:			This software is provided by the copyright holder "as is" and any 
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

#include <avr/eeprom.h>
#include "Arduino.h"
#include "Hacklace.h"
#include "Hacklace_AppEngine.h"
#include "Hacklace_App.h"


extern Hacklace_AppEngine HL;


/*************
 * constants *
 *************/

#ifdef APP_NAME
	#undef APP_NAME
	#undef APP_CLASSNAME
#endif

#define APP_NAME		AnimationApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)


/*********
 * class *
 *********/

class APP_CLASSNAME : public Hacklace_App
{
	public:
		const unsigned char* setup(const unsigned char* ee);
		void run();
		void finish();

	private:
		static byte brightness;
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::brightness = 0;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	byte ch;
	
	ch = eeprom_read_byte(ee++);		// set speed and delay
	HL.setScrollSpeed(swap(ch), ch);
	ch = eeprom_read_byte(ee++);		// set direction and increment
	HL.setScrollMode(ch, ch);
	HL.setSpacing(1);
	HL.setBrightness(brightness);
	// display text / animation from EEPROM
	return( (const unsigned char*) HL.printString_EE( (const char*) ee) );

}


void APP_CLASSNAME::run()
{
	if (HL.button == BTN2_RELEASED) {
		HL.buttonAck();
		brightness++;
		if (brightness > DISP_MAX_BRIGHT) { brightness = 0; }
		HL.setBrightness(brightness);
	}
}


void APP_CLASSNAME::finish()
{
}


