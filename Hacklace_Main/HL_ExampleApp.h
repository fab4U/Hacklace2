/*
 * HL_ExampleApp.cpp
 *
 */ 

/**********************************************************************************

Description:	Hacklace Example App

		Three steps to success:
					
		1. Enter the app name under "constants". In your Arduino program 
		include this file and register your app under the given name.
					
		2. Add member variables to the class as you need. Use static variables
		to preserve content between (run-) calls. Static variables must be 
		defined under "static class variables".
					
		3. Implement the functionality of your app. Method "setup" is called 
		once to initialize your app. The "run" method is called periodically 
		(every 10 ms) and will do most of the show. When you switch to the 
		next app the "finish" method is called once to clean-up your app.
					
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

// ##### enter your app name here #####
#define APP_NAME		ExampleApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)	// do not change


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
		// Use static variables if you want to preserve their content between calls.
		// ##### enter your member variables here... #####
		static byte timer;
		static byte brightness;
		static byte brightness_save;
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/
// Note:  Each static variable name must be prefixed with "APP_CLASSNAME::"

// ##### list your static variables here... #####
byte 	APP_CLASSNAME::timer;
byte 	APP_CLASSNAME::brightness;
byte 	APP_CLASSNAME::brightness_save;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	// ee points to the byte after the app_id.
	// You can use ee to read parameters from the EEPROM.

	// ##### enter your setup code here... #####
	HL.setScrollSpeed(11, 0);				// speed, delay
	HL.setScrollMode(BIDIRECTIONAL, 1);
	HL.setSpacing(0);
	HL.printString_P(PSTR(" -=[]=- "));
	brightness_save = HL.getBrightness();	// remember brightness setting
	brightness = 0;
	timer = 0;

	// If you have read data from the EEPROM return a pointer to the byte just after the data.
	return( ee );
}


void APP_CLASSNAME::run()
{
	// run executes periodically
	
	// ##### enter your application code here... #####
	// increase brightness every 15 calls
	if (timer > 0) { timer--; }
	else {
		timer = 15;
		brightness++;
		if (brightness > DISP_MAX_BRIGHT) { brightness = 0; }
		HL.setBrightness(brightness);
	}
}


void APP_CLASSNAME::finish()
{
	// will be called before switching to the next app
	
	// ##### enter your cleanup code here... #####
	HL.setBrightness(brightness_save);		// restore former brightness
}



