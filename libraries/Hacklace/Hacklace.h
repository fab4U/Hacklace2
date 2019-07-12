/*
 * Hacklace.h
 *
 */ 

/**********************************************************************************

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


#ifndef HACKLACE_H_
#define HACKLACE_H_

#include "Arduino.h"
#include "Hacklace_App.h"


/*************
 * constants *
 *************/

// dot matrix display
#define DISP_COLS			8			// number of columns (range 1..8)
#define DISP_ROWS			8			// number of rows (range 1..8)

// display dimming
#define DISP_MAX_BRIGHT		3			// brightness range 0..DISP_MAX_BRIGHT
										// max. dimming is (1/DISP_MAX_DIM) of full brightness
#define DISP_MAX_DIM		(1<<DISP_MAX_BRIGHT)
#define DISP_FRAME_RATE		100			// number of dot matrix images per second
#define OCR1A_CYCLE_TIME	(int)(0.5 + F_CPU / (8.0 * DISP_FRAME_RATE * DISP_COLS * DISP_MAX_DIM))

// display memory
#define DISP_MAX			1000		// size of display memory in bytes (1 byte = 1 column)

// system timer
#define SYS_TIMER_FREQ  	100			// frequency of system timer (Hz)
#define OCR1B_CYCLE_TIME	(int)(0.5 + F_CPU / (8.0 * SYS_TIMER_FREQ))
#define T1_PRESC			2			// timer 1 prescaler value (1:8, do not change)

// i/o initialization
#define DDRB_INIT			0b00111111
#define PORTB_INIT			0b00111111
#define DDRC_INIT			0b00111111
#define PORTC_INIT			0b00111111
#define DDRD_INIT			0b11110000
#define PORTD_INIT			0b11111111
#define PORTD_ROW_MASK		0b11000000
#define PORTD_COL_MASK		0b00110000

// push buttons
#define BTN_LONGPRESS_DELAY	100					// number of system timer cycles

// bit masks for push buttons (do not change)
#define BUTTON1_BIT			2					// bit number of left push-button
#define BUTTON2_BIT			1					// bit number of right push-button
#define BTN_PORT			D					// letter of input port used for push-buttons
#define BUTTON1				(1<<BUTTON1_BIT)
#define BUTTON2				(1<<BUTTON2_BIT)
#define BTN_MASK			(BUTTON1|BUTTON2)	// mask to extract button state
#define PB_PRESS			(1<<5)				// 1 = pressed, 0 = released
#define PB_RELEASE			(0<<5)
#define PB_LONG				(1<<6)				// 1 = pb_timer has elapsed
#define PB_LONGPRESS		(PB_LONG|PB_PRESS)	// 1 = long press, 0 = short press
#define PB_ACK				(1<<7)				// acknowledge, 1 = key event has been processed

// push button events (do not change)
#define BTN1_PRESSED		(BUTTON1 | PB_PRESS)
#define BTN1_RELEASED		(BUTTON1 | PB_RELEASE)
#define BTN1_LONGPRESSED	(BUTTON1 | PB_LONGPRESS)
#define BTN2_PRESSED		(BUTTON2 | PB_PRESS)
#define BTN2_RELEASED		(BUTTON2 | PB_RELEASE)
#define BTN2_LONGPRESSED	(BUTTON2 | PB_LONGPRESS)

// synchronization flags
#define COLUMN_SYNC			(1<<0)	// flag signalling that a new column has been displayed
#define SCROLL_SYNC			(1<<1)	// flag that is set if end of scrolling range has been reached
#define SYS_TIMER_SYNC		(1<<2)	// flag that is set with every system timer cycle

// pin definitions (do not change)
#define	RXD		RX
#define	TXD		TX
#define	BTN1	2
#define	BTN2	TX
#define	ROW1	8
#define	ROW2	9
#define	ROW3	10
#define	ROW4	11
#define	ROW5	12
#define	ROW6	13
#define	ROW7	6
#define	ROW8	7
#define	COL1	A0
#define	COL2	A1
#define	COL3	A2
#define	COL4	A3
#define	COL5	A4
#define	COL6	A5
#define	COL7	4
#define	COL8	5

// scrolling directions (do not change)
#define SCROLL_BACK			(1<<4)			// forward / backward bit mask (0 = forward)
#define SCROLL_BIDIR		(1<<5)			// unidirectional / bidirectional bit mask (0 = unidir)
#define SCROLL_OFF			(1<<6)			// scrolling on / off bit mask (0 = on)
#define NO_SCROLLING		(SCROLL_OFF)
#define FORWARD				0				// text moves from right to left
#define BACKWARD			SCROLL_BACK		// text moves from left to right
#define BIDIRECTIONAL		SCROLL_BIDIR	// text reverses direction

// string escape characters
#define RAW_MODE_CHAR		0x1F
// In "raw mode" the bytes are not interpreted as character codes but are directly
// written to the display memory. The byte after the RAW_MODE_CHAR specifies the 
// number of raw bytes that follow.

// special character codes
#define HL2_LOGO			0
#define HAPPY_SMILEY		1
#define SAD_SMILEY			2
#define SKULL				3
#define HEART				4
#define INVADER1			5
#define INVADER2			6
#define INVADER3			7
#define GHOST				8
#define FOX					9
#define NOTES				11
#define NOTE				12
#define GLASS				14
#define MOBILE				15
#define LIGHTNING			16
#define LIGHTBULB			17
#define MALE				20
#define FEMALE				21
#define PHONE				22
#define STICKFIG			23
#define DIAMOND				24
#define CLOCK				25
#define BELL				26
#define SPACE				32			// 3 column space
#define SPC1				0xA0		// 1 column space
#define SPC8				0x7F		// 8 column space
#define RAW					RAW_MODE_CHAR

// debugging features
//
// When the run-sleep-monitor is enabled pin PD3 will be high when the cpu is active 
// and low when the cpu is sleeping.
//#define	ENABLE_RUN_SLEEP_MONITOR


/**********
 * macros *
 **********/

// concatenation macros to create AVR port names from port letters (e. g. PORT(D) -> PORTD)
#define CCAT(a,b)			a ## b
#define CONCAT(a, b)		CCAT(a,b)
#define PIN(b)				CCAT(PIN,b)
#define PORT(b)				CCAT(PORT,b)
#define DDR(b)				CCAT(DDR,b)

// swapping upper and lower nibble of a byte
// usage: b=swap(a) or b=swap(b)
#define swap(x)                                            \
 ({                                                        \
    unsigned char __x__ = (unsigned char) x;               \
    asm volatile ("swap %0" : "=r" (__x__) : "0" (__x__)); \
    __x__;                                                 \
  })

// _BV implemented as a look-up table
const byte bit_value_table[8] PROGMEM = {1, 2, 4, 8, 16, 32, 64, 128};	// bit_value look-up table
#define BV_LOOKUP(bit)		pgm_read_byte(&bit_value_table[bit])


/***********
 * classes *
 ***********/

class Hacklace
{
	public:
		static byte displayMemory[DISP_MAX];
		static byte	button;				// button event

		static void initialize();
		static void copyToEeprom(const char* flash_ptr, unsigned int size);
		static void setBrightness(byte br);
		static byte getBrightness();
		static void setSpacing(byte spc);
		static void setScrollSpeed(byte speed, byte delay);
		static void setScrollMode(byte dir, byte inc);
		static void setCursor(unsigned int cur);
		static void cursorHome();
		static void setViewport(unsigned int vp);
		static void clearDisplay();
		static void printByte(byte byt);
		static void printImage_P(const byte* img);
		static void printChar(byte ch);
		static const char* printString(const char* st);
		static const char* printString_P(const char* st);
		static const char* printString_EE(const char* st);
		static void printMiniDigit(byte val, byte y);
		static byte getPixel(word x, byte y);
		static void setPixel(word x, byte y, byte pen);
		static void drawRect(word x1, byte y1, word x2, byte y2, byte pen);
		static void print0_99(byte val, byte y);
		static byte sysTimerHasElapsed();
		static byte scrollSync();
		static byte columnSync();
		static void disableDisplay();
		static void enableDisplay();
		static void run();
		static void goToSleep();
		static void powerDown();
		static void buttonAck();
		static void disableButton2();
		static void enableButton2();
		static void enableFreqCounter();
		static void disableFreqCounter();
		static void getFreqCounter(unsigned long* time, word* count);
		static float getFrequency();
		static float getPeriod();
		static word getInt1Count();
		inline static void outputColumn();
		inline static void setSysTimerFlag();
		inline static void int1Handler();
		inline static void t1overflowHandler();

	protected:
		static unsigned int	viewport;	// defines which section of the display memory appears 
										// on the dot matrix (viewport ... viewport+DISP_COLS-1)
		static byte		dimming;		// range 1..DISP_MAX_DIM with 1 = brightest
		static byte 	current_column;	// number of currently displayed column
		static unsigned int cursor;		// index of first free byte after current  
										// display memory content (0 = empty display)
		static byte		spacing;		// character spacing (number of empty colums between characters)
		static byte 	scroll_mode;	// lower nibble = scrolling increment (number of columns, 0 = off)
										// bit 4 = direction (0 = forward, 1 = backward)
										// bit 5 = bidirectional (0 = off, 1 = on)
		static byte		scroll_speed;	// scrolling speed (0 = fastest)
		static byte		scroll_delay;	// delay (number of scrolling steps) before scrolling cycle restarts
		static byte		delay_counter;	// counter for scroll delays (counting down to zero)
		static byte 	scroll_timer;
		static byte		btn_mask;		// mask to extract button state
		static byte		pb_timer;		// push button timer
		static volatile byte	sync_flags;
		static word		overfl_cnt;		// timer 1 overflow counter
		static word		tcnt_old;
		static word		int1_cnt;		// interrupt INT1 counter
		static unsigned long	interval_accu;	// accumulated intervals between INT1 interrupts

		static void scrollDisplay();
		static void sampleButtons();
		static const char* printStr(const char* st, byte src_mem_type);
		static byte readByte(const char* ptr, byte src_mem_type);
};


#endif /* HACKLACE_H_ */
