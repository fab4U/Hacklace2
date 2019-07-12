/*
 * HL_SpiralApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Spiral App

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

#define APP_NAME		SpiralApp
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
		static byte x;
		static byte y;
		static byte r;
		static byte pen;
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::x;
byte 	APP_CLASSNAME::y;
byte 	APP_CLASSNAME::r;
byte 	APP_CLASSNAME::pen;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	HL.setScrollMode(NO_SCROLLING, 1);
	x = 0;
	y = 0;
	r = 0;
	pen = 1;
	HL.displayMemory[0] = 1;					// set first dot
	return( ee );
}


void APP_CLASSNAME::run()
{
	if ( (y == r) && (x < (7 - r)) ) {			// top
		x++;
	} else
	if ( (x == (7 - r)) && (y < (7 - r)) ) {	// right
		y++;
	} else
	if ( (y == (7 - r)) && (x > r) ) {			// bottom
		x--;
	} else
	if ( (x == r) && (y > (r + 1)) ) {			// left
		y--;
	} else
	if ( r < 3 ) {								// next radius
		r++;
	} else {
		r = 0;  x = 0;  y = 0;
		pen ^= 1;								// invert pen
	}
	HL.setPixel(x, y, pen);
}


void APP_CLASSNAME::finish()
{
}


