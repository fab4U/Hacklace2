/*
 * HL_DownloadApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Download App

					Enables the user to download text and animations to the Hacklace 
					via the serial interface.
					All non-printing characters (charater code < 32) are ignored.
					
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

#define APP_NAME		DownloadApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

#define RESET_CHAR		0x1B		// <ESC>
// Sending the RESET_CHAR resets the receiver state machine. The RESET_CHAR always takes priority.
// Note: If you want to use the RESET_CHAR as an ordinary text character or as a data byte
// you cannot send it directly, because this would reset the receiver. Instead use the
// HEX_MODE_CHAR notation (see below).

#define HEX_MODE_CHAR	'\\'
// This feature allows for the input of an arbitrary byte values in hexadecimal form. 
// After receiving the HEX_MODE_CHAR the receiver is switched to hexadecimal mode; another 
// HEX_MODE_CHAR switches back to normal character mode.
// When in hexadecimal mode you can send a sequence of two-digit hexadecimal numbers 
// separated by one or more spaces. Non-hexadecimal characters will be ignored.
// Example: "This is normal text.\12 AB 3e ff D0   D1 D3 cC\ <- That was hexadecimal input."
// To use the HEX_BYTE_CHAR as a text character simply double it (e. g. '\\' to 
// display a single backslash).

#define COMMENT_CHAR		'#'
// If the COMMENT_CHAR is found directly after a HEX_MODE_CHAR the rest of the line is ignored.

// states of serial receiver state machine
#define RESET				0
#define IDLE				1
#define AUTH1				2		// first authentication byte received
#define AUTH2				3		// second authentication byte received
#define ENTER_CHAR			4
#define HEX_MODE			5
#define HEX_SEPARATOR		6
#define HEX_DIGIT			7
#define COMMENT				8
#define OVERFLOW			9
#define LOAD_DEFAULTS		10
#define SHOW_VERSION		11

#define AUTH1_CHAR			'H'
#define AUTH2_CHAR			'L'

#define DEFAULT1_CHAR		'R'
#define DEFAULT2_CHAR		'D'

#define VERSION1_CHAR		'V'
#define VERSION2_CHAR		'N'

const byte DownloadAppIcon[] PROGMEM =	{8, 0x11, 0x1F, 0x11, 0x0E, 0x00, 0xF8, 0x80, 0x80};
const byte error_icon[] PROGMEM =		{8, 0x1F, 0x15, 0x11, 0x00, 0xF8, 0xA8, 0x88, 0x00};


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
		static byte value;
		static byte* ee_write_ptr;
		
		static void interpreter(byte ch);
		static byte hex_value(byte ch);
};


APP_CLASSNAME APP_NAME;								// create an instance of the app class
const Hacklace_App* DownloadApp_ptr = &APP_NAME;	// pointer to the app instance


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::state;
byte	APP_CLASSNAME::value;
byte*	APP_CLASSNAME::ee_write_ptr;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	HL.printImage_P(DownloadAppIcon);
	ee_write_ptr = EE_START_ADDR;
	state = IDLE;
	HL.disableButton2();
	Serial.begin(2400);		// baud rate must be <= 2400
	
	return( ee );
}


void APP_CLASSNAME::run()
{
	while( Serial.available() ) {
		interpreter( Serial.read() );
	}
}


void APP_CLASSNAME::finish()
{
	Serial.end();
	HL.enableButton2();
}


void APP_CLASSNAME::interpreter(byte ch)
{
	// state machine that decodes the serial input stream
	if (ch == RESET_CHAR) { state = RESET; }
	HL.clearDisplay();
	if ((ch < 0x80) || (ch >= 0xA0)) { HL.printChar(ch); }	// print ch (exclude animations)

	switch (state) {
		case RESET:
			HL.clearDisplay();
			HL.printImage_P(DownloadAppIcon);
			ee_write_ptr = EE_START_ADDR;
			state = IDLE;
			break;

		// cases for initialization sequence
		case IDLE:
			if (ch == AUTH1_CHAR)	{ state = AUTH1; }
			else if (ch == DEFAULT1_CHAR)	{ state = LOAD_DEFAULTS; }
			else if (ch == VERSION1_CHAR)	{ state = SHOW_VERSION; }
			else					{ state = IDLE; }
			break;
		case AUTH1:
			if (ch == AUTH2_CHAR)	{ state = ENTER_CHAR; }
			else					{ state = IDLE; }
			break;
			
		// cases for writing to EEPROM
		case ENTER_CHAR:
			if (ch < ' ') {	break; }						// ignore non-printing characters
			if (ch == HEX_MODE_CHAR) {
				state = HEX_MODE;
				break;
			}
			eeprom_write_byte(ee_write_ptr++, ch);
			if (ee_write_ptr > (byte*)EE_END_ADDR) { state = OVERFLOW; }
			break;
		case HEX_MODE:
			if (ch == HEX_MODE_CHAR) {						// double HEX_MODE_CHAR ?
				eeprom_write_byte(ee_write_ptr++, ch);		// -> store as ordinary character
				if (ee_write_ptr > (byte*)EE_END_ADDR) { state = OVERFLOW; }
				else { state = ENTER_CHAR; }				// return to character mode
				break;
			}
			if (ch == COMMENT_CHAR) {						// is this a comment ?
				state = COMMENT;
				break;
			}
			value = 0;
			// no break -> fall through to next case
		case HEX_SEPARATOR:
			if (ch == HEX_MODE_CHAR) { 
				state = ENTER_CHAR;							// leave hex mode
				break;
			}
			if (hex_value(ch) > 15) { break; }				// ignore non-hexadecimal characters
			state = HEX_DIGIT;
			// no break -> fall through to next case
		case HEX_DIGIT:
			if (ch == HEX_MODE_CHAR) { 
				eeprom_write_byte(ee_write_ptr++, value);
				if (ee_write_ptr > (byte*)EE_END_ADDR) { state = OVERFLOW; }
				else { state = ENTER_CHAR; }				// leave hex mode
				break;
			}
			if (ch == ' ') {
				eeprom_write_byte(ee_write_ptr++, value);
				value = 0;
				if (ee_write_ptr > (byte*)EE_END_ADDR) { state = OVERFLOW; }
				else { state = HEX_SEPARATOR; }				// ignore further separator characters
				break;
			}
			if (hex_value(ch) <= 15) {
				value <<= 4;
				value += hex_value(ch);
			}
		case COMMENT:
			if ((ch == 13) || (ch == 10)) {
				state = ENTER_CHAR;
			}
			break;
		case OVERFLOW:
			HL.clearDisplay();
			HL.printImage_P(error_icon);
			break;

		case LOAD_DEFAULTS:
			if (ch == DEFAULT2_CHAR) {
				HL.copyToEeprom(ee_default, ee_default_size);
				HL.clearDisplay();
				HL.printChar(0x1F);							// print 'ok'
			}
			state = IDLE;
			break;
			
		case SHOW_VERSION:
			if (ch == VERSION2_CHAR) {
				HL.clearDisplay();
				HL.print0_99(HL_VERSION_MAJOR * 10 + HL_VERSION_MINOR, 2);
			}
			state = IDLE;
			break;
			
	}
}


byte APP_CLASSNAME::hex_value(byte ch)
{
	// Return the value of a single hexadecimal digit.
	// Non-hexadecimal characters will return values > 15.
	if (ch >= 'a') { ch -= ('a' - 'A'); }			// make upper case
	if (ch >= 'A') { return(ch - 'A' + 10); }		// map characters 'A'..'F' to 10..15
	if (ch <= '9') { return(ch - '0'); }			// map characters '0'..'9' to 0..9
	return(255);
}
