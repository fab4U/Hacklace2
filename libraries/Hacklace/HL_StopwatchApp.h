/*
 * HL_StopwatchApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Stopwatch

					Press right button to start / stop the watch.
					Hold right button pressed for two seconds to reset the watch.
					Accuracy is +/- 0.5 %

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

#define APP_NAME		StopwatchApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

// state machine states
#define	S_WAIT_ON_TITLE		0
#define	S_INIT				1
#define	S_STOPPED			2
#define	S_RUNNING			3


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
		static byte seconds;	// time of stopwatch (seconds)
		static byte tenths;		// time of stopwatch (tenths of a seconds)
		static byte state;
		static byte timer;		// 10 ms timer (= running at 100 Hz)
		
		void display_time();
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::seconds;
byte 	APP_CLASSNAME::tenths;
byte 	APP_CLASSNAME::state;
byte 	APP_CLASSNAME::timer;


/***********
 * methods *
 ***********/

void APP_CLASSNAME::display_time()
{
	HL.cursorHome();
	HL.print0_99(seconds, 0);
	if (tenths == 0)		{ }
	else if (tenths < 9)	{ HL.drawRect(0, 7, tenths-1, 7, 1); }
	else					{ HL.drawRect(0, 7, 7, 7, 1); HL.drawRect(7, 6, 7, 6, 1); }
}


const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	HL.setScrollSpeed(9, 6);
	HL.printString_P(PSTR(" Stopwatch  "));
	HL.scrollSync();						// clear sync flag
	state = S_WAIT_ON_TITLE;
	return( ee );
}


void APP_CLASSNAME::run()
{
	switch (state) {
		case S_WAIT_ON_TITLE:								//-------------------------
			if (HL.scrollSync()) { state = S_INIT; }		// wait until end of scrolling
			break;
		case S_INIT:										//-------------------------
			HL.clearDisplay();
			HL.setScrollMode(NO_SCROLLING, 1);
			timer = 9;
			seconds = 0;
			tenths = 0;
			display_time();
			state = S_STOPPED;
			break;
		case S_STOPPED:										//-------------------------
			if (HL.button == BTN2_RELEASED) {
				HL.buttonAck();
				state = S_RUNNING;
			}
			if (HL.button == BTN2_LONGPRESSED) {
				HL.buttonAck();
				timer = 9;
				seconds = 0;
				tenths = 0;
				display_time();
			}
			break;
		case S_RUNNING:										//-------------------------
			if (HL.button == BTN2_RELEASED) {
				HL.buttonAck();
				state = S_STOPPED;
			}
			if (HL.button == BTN2_LONGPRESSED) {
				HL.buttonAck();
				timer = 9;
				seconds = 0;
				tenths = 0;
				display_time();
				state = S_STOPPED;
			}
			if (timer) {
				timer--;
			}
			else {
				timer = 9;
				if (tenths == 9) {
					tenths = 0;
					if (seconds < 99) { seconds++; }
				}
				else { tenths++; }
				display_time();
			}
			break;
	}
}


void APP_CLASSNAME::finish()
{
}


