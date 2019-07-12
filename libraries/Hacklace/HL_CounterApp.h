/*
 * HL_CounterApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Counter App

					Press right button to count. Press and hold right button to reset
					counter.

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

#define APP_NAME		CounterApp
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
		static byte count;
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::count;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	HL.setScrollMode(NO_SCROLLING, 1);
//	count = 0;
	HL.print0_99(count, 1);
	return( ee );
}


void APP_CLASSNAME::run()
{
	if (HL.button == BTN2_RELEASED) {
		HL.buttonAck();
		if (count < 99) { count++; }
		HL.cursorHome();
		HL.print0_99(count, 1);
	}
	if (HL.button == BTN2_LONGPRESSED) {
		HL.buttonAck();
		count = 0;
		HL.cursorHome();
		HL.print0_99(count, 1);
	}
}


void APP_CLASSNAME::finish()
{
}


