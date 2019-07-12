/*
 * HL_ThermometerApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Thermometer App

					Using an NTC to measure temperatures.
					
					Connect NTC with nominal 10k @ 25 °C between pin 1 and pin 2 of 
					sensor port. Connect a 1k5 resistor between pin 4 and pin 5.
					These values are optimized for a temperature range of 30 ... 100 °C, 
					which is well suited to control the temperature of electronic 
					equipment. For other applications use the spreadsheet to adapt 
					the component values.
										
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

#define APP_NAME		ThermometerApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

#define INTERVAL		1		// time (seconds) between two measurements (range 0.01 ... 600)

// calibration
#define OFFSET			9.425
#define SLOPE			0.128

// states
#define ST_SHOW_TITLE	0
#define ST_MEASURING	1


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
		static byte state;
		static word timer;
};


APP_CLASSNAME APP_NAME;			// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::state;
word 	APP_CLASSNAME::timer;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	state = ST_SHOW_TITLE;
	timer = 0;
	HL.setScrollSpeed(8, 6);
	HL.printString_P(PSTR(" Thermometer  "));
	HL.scrollSync();						// clear sync flag
	pinMode(3, OUTPUT);
	analogReference(DEFAULT);
	return( ee );
}


void APP_CLASSNAME::run()
{
	float	temperature;					// temperature in °C
	word	ana;							// adc reading
	
	switch (state) {
		case ST_SHOW_TITLE:
			if (HL.scrollSync()) {			// wait until end of scrolling occurs
				HL.setScrollMode(NO_SCROLLING, 1);
				state = ST_MEASURING;
			}
		break;
		case ST_MEASURING:
											// to avoid disturbances of the a/d conversion
			if (timer) { timer--; }
			else {
				timer = ((word) (INTERVAL / 0.01)) - 1;
											// to avoid disturbances of the a/d conversion
				HL.disableDisplay();		// turn display off
				digitalWrite(3, LOW);		// turn sensor on
				delay(1);					// wait some time for the system to settle
				ana = analogRead(A6);
				digitalWrite(3, HIGH);		// turn sensor off
				HL.enableDisplay();			// turn display on again
				temperature = OFFSET + SLOPE * (float) ana;
				HL.cursorHome();
				HL.print0_99((byte) (temperature + 0.5), 3);
			}
		break;
	}
}


void APP_CLASSNAME::finish()
{
	pinMode(3, INPUT);
}


