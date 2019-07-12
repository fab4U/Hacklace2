/*
 * HL_DisplayTestApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Display Test App

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

#define APP_NAME		DisplayTestApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

#define R_MAX			7


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
		static byte timer;
		static byte timer_reload;
		static byte state;
		static byte brightness;
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::timer;
byte 	APP_CLASSNAME::timer_reload;
byte 	APP_CLASSNAME::state;
byte 	APP_CLASSNAME::brightness;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	HL.setScrollMode(NO_SCROLLING, 1);
	timer_reload = 10;						// defines speed of pattern changes
	timer = timer_reload;					// start timer
	state = 0;
	brightness = 0;
	HL.clearDisplay();
	HL.setBrightness(brightness);
	return( ee );
}


void APP_CLASSNAME::run()
{
	if (timer) {
		timer--;
		return;
	}

	HL.clearDisplay();
	if (state < 16) {
		if (state == 0) {
			if (brightness == 0) { brightness = DISP_MAX_BRIGHT; }
			else { brightness = 0; }
			HL.setBrightness(brightness);
		}
		if (state < 8) {
			HL.drawRect(state, 0, state, 7, 1);
		}
		else {
			HL.drawRect(0, state-8, 7, state-8, 1);
		}
		timer = timer_reload;
		state++;
	}
	else {
		HL.drawRect(0, 0, 7, 7, 1);
		timer = 5 * timer_reload;
		state = 0;
	}
}


void APP_CLASSNAME::finish()
{
	HL.setBrightness(0);
}


