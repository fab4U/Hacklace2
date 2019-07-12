/*
 * Hacklace_AppEngine.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace 2 AppEngine
					
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
 

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "Arduino.h"
#include "Hacklace.h"
#include "Hacklace_AppEngine.h"


/************
 * Instance *
 ************/

Hacklace_AppEngine HL;		// instantiate a Hacklace_AppEngine


/*************
 * variables *
 *************/

// app registry
extern const Hacklace_App* const app_registry[MAX_APPS];
extern const Hacklace_App* DownloadApp_ptr;

// default eeprom content
const byte ee_default[] PROGMEM = {
	ANIMATION, 0x76, 0x01, ' ', 'H', 'a', 'c', 'k', 'l', 'a', 'c', 'e', '2', ' ', SKULL, SPC1, 0,
	ANIMATION, 0x85, 0x01, ' ', 'A', 'r', 'd', 'u', 'i', 'n', 'o', '-', 'c', 'o', 'm', 'p', 'a', 't', 'i', 'b', 'l', 'e', SPC8, 0,
	ANIMATION, 0x85, 0x01, ' ', 'I', ' ', HEART, ' ', 'e', 'l', 'e', 'c', 't', 'r', 'o', 'n', 'i', 'c', 's', '!', ' ', ' ', 0,
	ANIMATION, 0x86, 0x01, ' ', 'H', 'i', 'g', 'h', ' ', 'V', 'o', 'l', 't', 'a', 'g', 'e', ' ', LIGHTNING, 0,
	ANIMATION, 0x54, 0x08, SPC1, SPC1, HEART, SPC8, SPC1, SPC1, HEART, SPC8, 0,	// heartbeat
	ANIMATION, 0x83, 0x21, 5, ' ', 6, ' ', 7, ' ', 8, 0,	// monster
	ANIMATION, 0xB0, 0x08, ANI(1), 0,						// fire
	ANIMATION, 0xA9, 0x01, ANI(2), 0,						// plug & socket
	ANIMATION, 0x8B, 0x08, ANI(3), 0,						// arrow
	ANIMATION, 0xA9, 0x18, SPC8, ANI(4), SPC8, 0,			// ball
	ANIMATION, 0xA8, 0x01, ANI(5), 0,						// ECG
	ANIMATION, 0x8A, 0x08, ANI(6), 0,						// droplet
	ANIMATION, 0x88, 0x01, 0x7F, ANI(7), 0x7F, 0,			// train
	ANIMATION, 0x88, 0x08, ANI(8), 0,						// pong
	ANIMATION, 0x35, 0x08, ANI(9), 0,						// wink
	ANIMATION, 0x7A, 0x08, ANI(10), 0x7F, 0,				// TNT
	ANIMATION, 0x14, 0x08, ANI(11), 0,						// house
	ANIMATION, 0x57, 0x08, ANI(12), 0,						// creeper
	ANIMATION, 0xA5, 0x08, ANI(13), 0,						// tunnel
	ANIMATION, 0x36, 0x08, ANI(14), 0,						// snow
	ANIMATION, 0x50, 0x08, ANI(15), 0,						// dancer
	ANIMATION, 0x36, 0x08, ANI(16), 0,						// fish
	ANIMATION, 0x80, 0x08, ANI(17), 0,						// radar
	ANIMATION, 0xA0, 0x08, ANI(18), 0,						// propeller
	ANIMATION, 0x79, 0x08, ANI(19), 0,						// flat cat
	ANIMATION, 0xA0, 0x08, ANI(20), 0,						// squares
	ANIMATION, 0x35, 0x08, ANI(21), ANI(22), 0,				// Tetris
	ANIMATION, 0x67, 0x08, ANI(23), 0,						// rocket
	ANIMATION, 0x90, 0x08, ANI(24), 0,						// spectrum
	ANIMATION, 0xA0, 0x08, ANI(25), 0,						// checkers
	ANIMATION, 0x79, 0x08, ANI(26), 0,						// headbang
//	ANIMATION, 0x5A, 0x08, ANI(31), 0,						// French Revolution
	EXAMPLE_APP,
	SPIRAL,
	COUNTER,
	GAME_OF_LIFE, 32, 15, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,		// glider
	GAME_OF_LIFE,  5, 50, 0x00, 0x24, 0x5A, 0x24, 0x24, 0x5A, 0x24, 0x00,		// oktagon
	GAME_OF_LIFE,  6, 10, 0x00, 0x04, 0x6C, 0x38, 0x1C, 0x36, 0x20, 0x00,		// pulsate
//	GAME_OF_LIFE, 32, 12, 0x00, 0x04, 0x05, 0x06, 0x10, 0x60, 0x30, 0x00,		// mix
//	GAME_OF_LIFE, 36, 12, 0x00, 0x00, 0x00, 0x10, 0x38, 0x08, 0x00, 0x00,		// grow
//	GAME_OF_LIFE,  4, 30, 0x00, 0x42, 0x3C, 0x24, 0x24, 0x3C, 0x42, 0x00,		// 4 cycle
//	GAME_OF_LIFE, 12, 15, 0x00, 0x00, 0x24, 0x2C, 0x20, 0x00, 0x00, 0x00,		// bubble
//	GAME_OF_LIFE, 16, 15, 0x50, 0x08, 0x08, 0x48, 0x38, 0x00, 0x00, 0x00,		// spaceship
	STOPWATCH,
	VOLTMETER, 39,							// needs extra hardware (resistor)
	FREQMETER,
	JUKEBOX,								// needs extra hardware (piezo buzzer + 100nF)
	THERMOMETER,							// needs extra hardware (resistor + thermistor)
	CODE_LOCK, 2, 0x12, 0x34, 0x99, 0x99,	// needs extra hardware (keypad + servo)
	SPEEDOMETER, 66, 8,						// needs extra hardware (reed relais + magnet)
											// circumference = 2114 mm (26" wheel, ETRTO 54-559)
	BATT_MONITOR,							// needs extra hardware (2 resistors)
	DISPLAY_TEST,
	END_OF_LIST
};
const unsigned int ee_default_size = sizeof(ee_default);

// static class variables
const unsigned char*	Hacklace_AppEngine::ee_ptr;
Hacklace_App*			Hacklace_AppEngine::app;					// pointer to current app


/***********
 * methods *
 ***********/

void Hacklace_AppEngine::initialize()
{
	Hacklace::initialize();

	printChar(HL2_LOGO);
	if (eeprom_read_byte(EE_START_ADDR) == 0xFF) {			// EEPROM empty?
		copyToEeprom(ee_default, sizeof(ee_default));		// -> restore default EEPROM content
		_delay_ms(1000);
	}

	app = (Hacklace_App*) pgm_read_word(&app_registry[RESET_APP]);	// get ResetApp
	if (app) {
		cursorHome();
		app->setup((const unsigned char*)EE_RESET_PARAMS);
	}
	ee_ptr = EE_START_ADDR;
}


void Hacklace_AppEngine::copyToEeprom(const byte* flash_ptr, unsigned int size)
{
	// Copy memory content from flash to EEROM.
	
	unsigned int i;
	byte ch;
	byte* ee_ptr;

	ee_ptr = EE_START_ADDR;
	for(i = 0; i < size; i++) {
		ch = pgm_read_byte(flash_ptr++);
		eeprom_write_byte(ee_ptr++, ch);
	}
}


Hacklace_App* Hacklace_AppEngine::getApp(byte app_id)
{
	if (app_id >= MAX_APPS) { return (NULL); }
	else { return ( (Hacklace_App*) pgm_read_word(&app_registry[app_id]) ); }
}


void Hacklace_AppEngine::resetApp()
{
	// Terminate the current app and set first app to be the next one.
	
	if (app) { app->finish(); }
	app = NULL;
	clearDisplay();
	ee_ptr = EE_START_ADDR;
}


void Hacklace_AppEngine::nextApp()
{
	// Switch to the next app as defined by the app_list in EEPROM.
	
	byte app_id;

	if (app) { app->finish(); }				// finish old app
	clearDisplay();
	setSpacing(1);
	setScrollSpeed(7, 7);
	setScrollMode(FORWARD, 1);
	app_id = eeprom_read_byte(ee_ptr++);	// read new app_id
	app = getApp(app_id);					// get pointer to app object
	if (app) {
		ee_ptr = app->setup(ee_ptr);
	} else {
		printString_P(PSTR("bad ID"));
	}

	app_id = eeprom_read_byte(ee_ptr);		// peek next app_id
	if (app_id == END_OF_LIST) {			// end of list?
		ee_ptr = EE_START_ADDR;				// -> restart from beginning
	}
}


void Hacklace_AppEngine::enterPowerDown()
{
	resetApp();										// terminate current app
	printChar(SAD_SMILEY);  _delay_ms(500);
	clearDisplay();
	while ((PIN(BTN_PORT) & BUTTON1) == 0) {}		// wait for button to be released
	_delay_ms(20);									// wait until bouncing has decayed
	powerDown();									// ---------- sleeping ----------
	while ((PIN(BTN_PORT) & BUTTON1) == 0) {}		// wait for button to be released
	_delay_ms(20);									// wait until bouncing has decayed
	
	printChar(HAPPY_SMILEY);
	if (PIN(BTN_PORT) & BUTTON2) {					// button2 not pressed
		_delay_ms(500);
		nextApp();									// start app
	} else {										// button2 pressed
		while ((PIN(BTN_PORT) & BUTTON2) == 0) {}	// wait for button to be released
		_delay_ms(20);								// wait until bouncing has decayed
		clearDisplay();
		app = (Hacklace_App*) DownloadApp_ptr;
		app->setup(EE_START_ADDR);					// enter download mode
	}
}


void Hacklace_AppEngine::run()
{
	Hacklace::run();
	if (app) { app->run(); }
	if (button == BTN1_RELEASED) {					// switch to next app
		buttonAck();
		nextApp();
	}
	if (button == BTN1_LONGPRESSED) {				// turn Hacklace off
		buttonAck();
		enterPowerDown();
	}
}


