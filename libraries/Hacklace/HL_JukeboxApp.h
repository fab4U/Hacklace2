/*
 * HL_JukeboxApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Jukebox App

					Plays a little melody.

					Connect a small passive piezo buzzer to the sensor port (pin 3 and 5).
										
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


extern Hacklace_AppEngine HL;


/*************
 * constants *
 *************/

#ifdef APP_NAME
	#undef APP_NAME
	#undef APP_CLASSNAME
#endif

#define APP_NAME		JukeboxApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

#define SPEAKER			3		// connect speaker to pin 3 (= PD3)

#define BPM				240		// beats per minute (range 3..240)
#define BEAT_DURATION	(byte)(0.5 + 6000.0 / ((float)BPM * 8.0))

// states
#define SHOW_TITLE	0
#define NEW_MELODY	1
#define PLAYING		2

// frequencies in Hz
#define NOTE_C7		2093
#define NOTE_CS7	2217
#define NOTE_D7		2349
#define NOTE_DS7	2489
#define NOTE_E7		2637
#define NOTE_F7		2794
#define NOTE_FS7	2960
#define NOTE_G7		3136
#define NOTE_GS7	3322
#define NOTE_A7		3520
#define NOTE_AS7	3729
#define NOTE_B7		3951

// notes (German names)
#define C	0x00
#define Cis	0x10	// = C#
#define Des	0x10	// = Db
#define D	0x20
#define Dis	0x30
#define Es	0x30
#define E	0x40
#define F_	0x50	// "F" is already defined so we use "F_" instead
#define Fis	0x60
#define Ges	0x60
#define G	0x70
#define Gis	0x80
#define As	0x80
#define A	0x90
#define Ais	0xA0
#define B	0xA0
#define H	0xB0
#define P	0xC0	// special note to represent a rest
#define END_OF_MELODY	0xF0

// table of note frequencies
const word freq_table[] PROGMEM = { 	NOTE_C7, NOTE_CS7, 
										NOTE_D7, NOTE_DS7, 
										NOTE_E7, 
										NOTE_F7, NOTE_FS7, 
										NOTE_G7, NOTE_GS7, 
										NOTE_A7, NOTE_AS7, 
										NOTE_B7
									};

// melodies
// A note is entered as <note>+<octave>, <duration>.
// Example: "A+4, 8" means middle A (440 Hz) with duration of 1 beat (quarter note)

// Bach: Jesus bleibet meine Freude
const byte bach[]  PROGMEM = {
	G+4, 8, A+4, 8, H+4, 8, D+5, 8, C+5, 7, P, 1, C+5, 8, E+5, 8, D+5, 7, P, 1, D+5, 8, G+5, 8, 
	Fis+5, 8, G+5, 8, D+5, 8, H+4, 8, G+4, 8, A+4, 8, H+4, 8, C+5, 8, D+5, 8, E+5, 8, D+5, 8, 
	C+5, 8, H+4, 8, A+4, 8, H+4, 8, G+4, 8, Fis+4, 8, G+4, 8, A+4, 8, D+4, 8, Fis+4, 8, A+4, 8, 
	C+5, 8, H+4, 8, A+4, 8, H+4, 8,
	END_OF_MELODY
};

const byte jingle_bells[]  PROGMEM = {
	E+4, 7, P, 1, E+4, 7, P, 1, E+4, 15, P, 1, E+4, 7, P, 1, E+4, 7, P, 1, E+4, 15, P, 1, 
	E+4, 8, G+4, 8, C+4, 8, D+4, 8, E+4, 32, 
	F_+4, 7, P, 1, F_+4, 7, P, 1, F_+4, 7, P, 1, F_+4, 7, P, 1, F_+4, 8, E+4, 7, P, 1, 
	E+4, 11, P, 1, E+4, 3, P, 1, E+4, 8, D+4, 7, P, 1, D+4, 8, E+4, 8, D+4, 16, G+4, 16,
	E+4, 7, P, 1, E+4, 7, P, 1, E+4, 15, P, 1, E+4, 7, P, 1, E+4, 7, P, 1, E+4, 15, P, 1, 
	E+4, 8, G+4, 8, C+4, 8, D+4, 8, E+4, 32, 
	F_+4, 7, P, 1, F_+4, 7, P, 1, F_+4, 7, P, 1, F_+4, 7, P, 1, F_+4, 8, E+4, 7, P, 1, 
	E+4, 7, P, 1, E+4, 8, G+4, 7, P, 1, G+4, 8, F_+4, 8, D+4, 8, C+4, 24, P, 8,	
	END_OF_MELODY
};

const byte chromatic_scales[]  PROGMEM = {
	C+3, 4, D+3, 4, E+3, 4, F_+3, 4,  G+3, 4,  A+3, 4,  H+3, 4,  C+4, 4, P, 16,
	C+4, 4, D+4, 4, E+4, 4, F_+4, 4,  G+4, 4,  A+4, 4,  H+4, 4,  C+5, 4, P, 16,
	C+5, 4, D+5, 4, E+5, 4, F_+5, 4,  G+5, 4,  A+5, 4,  H+5, 4,  C+6, 4, P, 16,
	END_OF_MELODY
};

// a list of all available melodies
const byte* const melody[]  PROGMEM = {
	bach, jingle_bells, chromatic_scales
};

#define NUM_OF_MELODIES	(sizeof(melody) / sizeof(byte*))

// the string that is shown during playing
const char animation_str[]  PROGMEM = {
	SPACE, NOTES, NOTE, (char)SPC1, NOTE, SPACE, NOTES, SPACE, NOTE, SPC8, 0
};


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
		static byte timer;
		static byte melody_index;
		static byte* note_ptr;
};


APP_CLASSNAME APP_NAME;			// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::state;
byte 	APP_CLASSNAME::timer;
byte 	APP_CLASSNAME::melody_index;
byte* 	APP_CLASSNAME::note_ptr;


/***********
 * methods *
 ***********/

word get_frequency (byte note)
// calculate the frequency of a note
// upper nibble encodes note name, lower nibble encodes octave
{
	byte i;
	word freq;
	
	i = note >> 4;							// extract note name
	if (i > 11) { return(0); }				// this is not a note but a rest
	freq = pgm_read_word(&freq_table[i]);	// get frequency
	for (i = (note & 0x07); i < 7; i++) {
		if (i == 6) freq++;					// rounding to next integer
		freq >>= 1;							// divide frequency by 2
	}
	return(freq);
}


const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	state = SHOW_TITLE;
	timer = 0;
	melody_index = 0;
	HL.setScrollSpeed(8, 6);
	HL.printString_P(PSTR(" Jukebox  "));
	HL.scrollSync();						// clear sync flag
	return( ee );
}


void APP_CLASSNAME::run()
{
	byte	note, duration;
	
	switch (state) {
		case SHOW_TITLE:
			if (HL.scrollSync()) {			// wait until end of scrolling occurs
				HL.clearDisplay();
				HL.printString_P(animation_str);
				state = NEW_MELODY;
			}
			break;
			
		case NEW_MELODY:
			note_ptr = (byte*) pgm_read_word(&melody[melody_index]);
			state = PLAYING;
			break;
			
		case PLAYING:
			if (timer) { timer--; }
			else {
				note = pgm_read_byte(note_ptr++);
				if (note == END_OF_MELODY) {
					note_ptr = (byte*) pgm_read_word(&melody[melody_index]);	// restart from beginning
					note = pgm_read_byte(note_ptr++);
				}
				if (note < P) { 
					pinMode(SPEAKER, OUTPUT);
					tone(SPEAKER, get_frequency(note));		// play note
				}
				else {
					pinMode(SPEAKER, INPUT);				// mute speaker
					noTone(SPEAKER);						// play rest
				}
				duration = pgm_read_byte(note_ptr++);
				timer = duration * BEAT_DURATION - 1;
			}
			
			if (HL.button == BTN2_RELEASED) {
				HL.buttonAck();
				if (melody_index < (NUM_OF_MELODIES - 1))	{ melody_index++; }
				else										{ melody_index = 0; }
				state = NEW_MELODY;
			}
			break;
	}
}


void APP_CLASSNAME::finish()
{
	noTone(SPEAKER);
	pinMode(SPEAKER, INPUT);
}


