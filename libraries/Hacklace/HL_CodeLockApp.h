/*
 * HL_CodeLockApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Code Lock App

					Implementation of a four digit code lock. 
					
					App parameters:
					1 byte  = number of valid access codes
					n bytes = list of valid access Codes
					          Each code consists of 2 bytes which specify 4 digits
					          in BCD representation. E. g. code "1234" must be stored 
					          as bytes 0x12 and 0x34.
					Key '*' clears the code. If in unlocked state returns to locked state. 
					Key '#' grants access if the entered code was correct.
					
					The lock output can either be a simple on/off output for switching 
					an electro magnet or a pwm signal for controlling a servo.
					For a simple on/off output set
					   SERVO_LOCKED   =   0 (  0 % duty cycle)
					   SERVO_UNLOCKED = 255 (100 % duty cycle)
					For a standard rc servo these values should be in the range of 
					approximately 10..30.

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
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "Hacklace.h"
#include "Hacklace_AppEngine.h"
#include "Hacklace_App.h"
#include "DecodeKeys.h"


extern Hacklace_AppEngine HL;


/*************
 * constants *
 *************/

#ifdef APP_NAME
	#undef APP_NAME
	#undef APP_CLASSNAME
#endif

#define APP_NAME		CodeLockApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

// state machine states
#define	S_WAIT_ON_MSG		0
#define	S_START				1
#define	S_WAIT_ON_RELEASE	2
#define	S_WAIT_ON_RELEASE2	3
#define	S_WAIT_ON_KEY		4
#define	S_DELAY_INPUT		5
#define	S_UNLOCKED			6

#define MAX_TRIALS			3		// maximum number of trials
#define MAX_TRIALS_DELAY	21		// delay (seconds) after maximum number of trials (max. 600s)
#define TRIAL_DELAY_COUNT	(word)(MAX_TRIALS_DELAY*100)

#define UNLOCK_TIME			30		// duration (seconds) of unlocking (max. 600s)
#define UNLOCK_TIME_COUNT	(word)(UNLOCK_TIME * 100)

// pwm settings
// pwm frequency is approx. 61 Hz
// pwm duty cylcle range is 0..255 with 0 = 0 % (constant off) and 255 = 100 % (constand on)
// for servos choose a duty cycle value within 12..32
#define SERVO_LOCKED		29		// pwm value for servo in locked position (range 0..255)
#define SERVO_UNLOCKED		10		// pwm value for servo in unlocked position (range 0..255)

//#define PANZERKNACKER		// enables texts messages for "Panzerknacker" game

// message strings
#ifdef PANZERKNACKER
#define MSG_APP_NAME		" Panzerknacker  "		// appears once when the app is selected
#define MSG_PROMPT			" Knack mich...  "		// promts user to enter code
#define MSG_OPEN			" GEWONNEN!  "			// message if lock was opened
#define MSG_WAIT			" Diesmal hat's leider nicht geklappt. Mehr Gl\xFC\x63k beim n\xE4\x63hsten mal!  "	// message if limit for number of trials is reached
#define MSG_WRONG			" \x02  "				// message if entered code was wrong (sad smiley)
#define MSG_LOCKING			" locking  "			// message if code is locking
#else
#define MSG_APP_NAME		" Code-Lock  "		// appears once when the app is selected
#define MSG_PROMPT			" enter code  "		// promts user to enter code
#define MSG_OPEN			" open  "			// message if lock was opened
#define MSG_WAIT			" wait  "			// message if limit for number of trials is reached
#define MSG_WRONG			" wrong code  "		// message if entered code was wrong
#define MSG_LOCKING			" locking  "		// message if code is locking
#endif

const byte key_icon[9] PROGMEM = {8, 0x00, 0x7e, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x00};

typedef union {
	uint16_t u16;
	struct {
		uint8_t low;
		uint8_t high;
	};
} uint8_16;


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
		static const unsigned char* access_code_ptr;	// EEPROM address to list of access codes
		static byte state;
		static byte trial;
		static word code;
		static word timer;
		
		boolean check_code(word code);
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

const unsigned char* 	APP_CLASSNAME::access_code_ptr;
byte 	APP_CLASSNAME::state;
byte 	APP_CLASSNAME::trial;
word 	APP_CLASSNAME::code;
word 	APP_CLASSNAME::timer;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	byte number_of_codes;

	// initialise lock output	
	TCNT2  = 0;
	OCR2B  = SERVO_LOCKED;
	TCCR2A = (2<<COM2B0)|(1<<WGM20);	// phase correct pwm mode, non-inverting pwm on OC2B
	TCCR2B = (6<<CS20);					// prescaler = 1:256, start timer
	pinMode(3, OUTPUT);

	access_code_ptr = ee;
	analogReference(DEFAULT);
	HL.setScrollSpeed(9, 6);
	HL.printString_P(PSTR(MSG_APP_NAME));
	HL.scrollSync();						// clear sync flag
	number_of_codes = eeprom_read_byte(ee++);
	trial = 0;
	state = S_WAIT_ON_MSG;
	return( ee + 2 * number_of_codes);
}


void APP_CLASSNAME::run()
{
	word ana;
	byte key;

	switch(state) {
		case S_WAIT_ON_MSG:									//-------------------------
			if (HL.scrollSync()) { state = S_START; }		// wait until end of scrolling
			break;
		case S_START:										//-------------------------
			code = 0;
			HL.clearDisplay();
			HL.printString_P(PSTR(MSG_PROMPT));
			state = S_WAIT_ON_RELEASE2;
			break;
		case S_WAIT_ON_RELEASE:								//-------------------------
			if (decodeKeys(analogRead(A6)) == KeyNONE) {
				HL.clearDisplay();
				state = S_WAIT_ON_KEY;
			}
			break;
		case S_WAIT_ON_RELEASE2:							//--- without clearing display
			if (decodeKeys(analogRead(A6)) == KeyNONE) {
				state = S_WAIT_ON_KEY;
			}
			break;
		case S_WAIT_ON_KEY:									//-------------------------
															// to avoid disturbances of the a/d conversion
			HL.disableDisplay();							// turn display off
			delay(1);										// wait some time for the system to settle
			ana = analogRead(A6);
			HL.enableDisplay();								// turn display on again
			key = decodeKeys(ana);
			if (key != KeyNONE) {
				HL.clearDisplay();
				HL.printImage_P(key_icon);
//				HL.printChar(key + '0');					// only for debugging purposes
				state = S_WAIT_ON_RELEASE;
			}
			if (key <= 9) {									// number key
				code <<= 4;
				code += key;
				state = S_WAIT_ON_RELEASE;
			}
			else if (key == 10) {							// key '*'
				state = S_START;
			}
			else if (key == 11) {							// key '#'
				trial++;
				HL.clearDisplay();
				if (check_code(code) == true) {
					HL.printString_P(PSTR(MSG_OPEN));
					OCR2B = SERVO_UNLOCKED;					// open lock
					timer = UNLOCK_TIME_COUNT;				// start timer
					state = S_UNLOCKED;
				}
				else if (trial >= MAX_TRIALS) {
					HL.printString_P(PSTR(MSG_WAIT));
					timer = TRIAL_DELAY_COUNT;				// start timer
					state = S_DELAY_INPUT;
				}
				else {
					HL.printString_P(PSTR(MSG_WRONG));
					state = S_WAIT_ON_MSG;
				}
				HL.scrollSync();							// clear sync flag
			}
			break;
		case S_DELAY_INPUT:									//-------------------------
			if (timer) { timer--; }
			else {
				trial = 0;
				state = S_START;
			}
			break;
		case S_UNLOCKED:									//-------------------------
			if (decodeKeys(analogRead(A6)) == 10) {			// key '*'
				timer = 0;									// stop timer
			}
			if (timer) { timer--; }
			else {
				HL.clearDisplay();
				HL.printString_P(PSTR(MSG_LOCKING));
				OCR2B = SERVO_LOCKED;						// close lock
				HL.scrollSync();							// clear sync flag
				trial = 0;
				state = S_WAIT_ON_MSG;
			}
			break;
	}
}


void APP_CLASSNAME::finish()
{
/*
	TCCR2A = 0;							// clear pwm mode
	TCCR2B = 0;							// stop timer
	pinMode(3, INPUT);
	digitalWrite(3, HIGH);
*/
}


boolean APP_CLASSNAME::check_code(word code)
// check if code is one of the allowed access codes
{
	byte					n;		// number of access codes
	uint8_16				c;
	const unsigned char*	ptr;

	ptr = access_code_ptr;
	for (n = eeprom_read_byte(ptr++); n > 0; n--) {
		c.high = eeprom_read_byte(ptr++);
		c.low  = eeprom_read_byte(ptr++);
		if (code == c.u16) { return (true); }
	}
	return (false);
}

