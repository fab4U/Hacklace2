/*
 * HL_SpeedometerApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Speedometer App

					Bicycle speedometer.
					First parameter is the wheel circumference in mm.
					(2 bytes, least significant byte first)
					
					Connect a reed relais between PD3 (sensor port pin 5) and 
					GND (sensor port pin 3). Attach a magnet to the wheel.
					
					Press right button to switch display between speed and total 
					distance.
					When displaying distance hold right button pressed to 
					reset distance to zero.

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

#define APP_NAME		SpeedometerApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

// state machine states
#define	S_WAIT_ON_MSG		0
#define	S_SPEED				1
#define	S_DIST				2

#define TIMEOUT				3	// time (s) with no pulses after which the speed is set to 0

#define DISTANCE_IN_M			// display distance in meters instead of kilometers
								// (out-comment to display kilometers)


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
		static byte  state;
		static word  timer;
		static float speed;
		static float circumference;
		static float distance;
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::state;
word 	APP_CLASSNAME::timer;
float 	APP_CLASSNAME::speed;
float 	APP_CLASSNAME::circumference;
float 	APP_CLASSNAME::distance;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	circumference = (float) eeprom_read_word((const word*) ee);		// read circumference (in mm)
	
	HL.enableFreqCounter();
	HL.setScrollSpeed(8, 6);
	HL.printString_P(PSTR(" Speedometer  "));
	HL.scrollSync();						// clear sync flag
	speed = 0;
	state = S_WAIT_ON_MSG;
	return( ee + sizeof(word) );
}


void APP_CLASSNAME::run()
{
	word			revolutions;
	unsigned long	interval;
	char			st[7];
	
	if (state == S_WAIT_ON_MSG) {
			timer = (TIMEOUT * 100);
			if (HL.scrollSync()) {		// wait until end of scrolling
				HL.setScrollMode(NO_SCROLLING, 1);
				HL.clearDisplay();
				HL.print0_99(speed, 1);
				state = S_SPEED;
			}
	}

	else {
		HL.getFreqCounter(&interval, &revolutions);
		if (revolutions > 0) {		// frequency is only valid if there has been
									// at least one revolution
			// More than 1 revolution pulse is considered as only 1 revolution,
			// which serves for debouncing the input signal.
									
			// increase total distance
			#ifdef DISTANCE_IN_M
			distance += (circumference / 1000.0);		// distance in m
			#else
			distance += (circumference / 1000000.0);	// distance in km
			#endif
							
			speed =  3600.0 / ((float) interval) * circumference;	// calculate speed  in km/h
			if (speed > 99.0) { speed = 99.0; }			// limit speed to 0..99 km/h
			timer = (TIMEOUT * 100);
		}
		else {
			if (timer) { timer--; }
			else {						// set speed to zero if there are no pulses for some time
				speed = 0;
			}
		}

		switch(state) {
			case S_SPEED:								//----- display speed -----
				if (revolutions > 0) {
					HL.cursorHome();
					HL.print0_99( (byte)(speed + 0.5), 1);
				}
				else {
					if (timer == 0) {
						HL.cursorHome();
						HL.print0_99(speed, 1);
					}
				}
				if (HL.button == BTN2_RELEASED) {
					HL.buttonAck();
					state = S_DIST;
					HL.clearDisplay();
					HL.setScrollMode(FORWARD, 1);
					HL.printString_P(PSTR("dist"));
					HL.scrollSync();					// clear sync flag
				}
				break;

			case S_DIST:								//----- display distance -----
				if (HL.scrollSync()) {		// wait until end of scrolling
					itoa((word)(distance + 0.5), st, 10);
					HL.cursorHome();
					#ifdef DISTANCE_IN_M
					HL.printString_P(PSTR("m "));
					#else
					HL.printString_P(PSTR("km "));
					#endif
					HL.printString(st);
				}
				if (HL.button == BTN2_RELEASED) {
					HL.buttonAck();
					state = S_SPEED;
					HL.setScrollMode(NO_SCROLLING, 1);
					HL.clearDisplay();
					HL.print0_99(speed, 1);
				}
				if (HL.button == BTN2_LONGPRESSED) {
					HL.buttonAck();
					HL.clearDisplay();
					HL.printString_P(PSTR("   "));
					distance = 0;
				}
				break;
		}

	}  /* of else */
}


void APP_CLASSNAME::finish()
{
	HL.disableFreqCounter();
}

