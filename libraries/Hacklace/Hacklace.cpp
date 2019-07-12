/*
 * Hacklace.cpp
 *
 */ 

/**********************************************************************************

Description:		Arduino library for operating the Hacklace(TM).
					The Hacklace library uses Timer1 which is running at 1 MHz.
					
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include "Arduino.h"
#include "Font.h"
#include "Hacklace.h"


// memory types
#define RAM			0
#define FLASH		1
#define EEPROM		2


/*************
 * constants *
 *************/

// speed and delay conversion
// Convert speed / delay parameters from range 0..15 to actual speed / delay values.
const byte dly_conv[16] PROGMEM = {0, 1, 2, 3, 4, 5, 6, 8, 10, 13, 16, 20, 25, 32, 40, 50};
const byte spd_conv[16] PROGMEM = {52, 43, 35, 28, 22, 17, 13, 10, 8, 6, 5, 4, 3, 2, 1, 0};

// small decimal digits
const byte minidigits[] PROGMEM = {	0x1F, 0x11, 0x1F,	// 0
									0x12, 0x1F, 0x10,	// 1
									0x1D, 0x15, 0x17,	// 2
									0x15, 0x15, 0x1F,	// 3
									0x07, 0x04, 0x1F,	// 4
									0x17, 0x15, 0x1D,	// 5
									0x1F, 0x15, 0x1D,	// 6
									0x01, 0x01, 0x1F,	// 7
									0x1F, 0x15, 0x1F,	// 8
									0x17, 0x15, 0x1F,	// 9
									0x1E, 0x05, 0x1E,	// A
									0x1F, 0x15, 0x1A,	// B
									0x0E, 0x11, 0x11,	// C
									0x1F, 0x11, 0x0E,	// D
									0x1F, 0x15, 0x11,	// E
									0x1F, 0x05, 0x01,	// F
								};


/**************
 * data types *
 **************/

typedef union {
	unsigned long u32;
	struct {
		word u16lo;
		word u16hi;
	};
} ulong_hilo;


/**************************
 * static class variables *
 **************************/

byte 			Hacklace::displayMemory[DISP_MAX];
byte 			Hacklace::current_column;	// number of currently displayed column
byte			Hacklace::dimming;			// range 1..DISP_MAX_DIM with 1 = brightest
unsigned int	Hacklace::viewport;			// defines which section of the display memory appears 
											// on the dot matrix (viewport ... viewport+DISP_COLS-1)
volatile byte	Hacklace::sync_flags;		// flags for synchronization
byte			Hacklace::button;			// button event
unsigned int 	Hacklace::cursor;			// index of first free byte after current  
											// display memory content (0 = empty display)
byte			Hacklace::spacing;			// character spacing (number of empty colums between characters)
byte 			Hacklace::scroll_mode;		// lower nibble = scrolling increment (number of columns, 0 = off)
											// bit 4 = direction (0 = forward, 1 = backward)
											// bit 5 = bidirectional (0 = off, 1 = on)
byte			Hacklace::scroll_speed;		// scrolling speed (0 = fastest)
byte			Hacklace::scroll_delay;		// delay (number of scrolling steps) before scrolling cycle restarts
byte			Hacklace::delay_counter;	// counter for scroll delays (counting down to zero)
byte 			Hacklace::scroll_timer;
byte			Hacklace::btn_mask;			// mask to extract button state
byte			Hacklace::pb_timer;			// push button timer

word			Hacklace::overfl_cnt;		// timer 1 overflow counter
word			Hacklace::tcnt_old;
word			Hacklace::int1_cnt;			// interrupt INT1 counter
unsigned long	Hacklace::interval_accu;	// accumulated intervals between INT1 interrupts


/***********
 * methods *
 ***********/

void Hacklace::initialize(void)
{
	sync_flags = 0;
	current_column = 0;
	scroll_timer = 0;
	delay_counter = 0;
	button = 0;
	btn_mask = BTN_MASK;
	
	// i/o initialization
	DDRB  = DDRB_INIT;
	PORTB = PORTB_INIT;
	DDRC  = DDRC_INIT;
	PORTC = PORTC_INIT;
#ifndef ENABLE_RUN_SLEEP_MONITOR
	DDRD  = DDRD_INIT;
#else
	DDRD  = DDRD_INIT | (1<<3);
#endif

	PORTD = PORTD_INIT;
	
	// timer1 initialization
	TCNT1 = 0;
	OCR1A = OCR1A_CYCLE_TIME;
	OCR1B = OCR1B_CYCLE_TIME;
	TCCR1A = 0;							// timer mode = normal
	TCCR1B = (T1_PRESC<<CS10);			// set prescaler
	TCCR1C = 0;
	TIFR1 = (1<<OCF1A)|(1<<OCF1B);		// clear compare-match interrupt flags
	TIMSK1 = (1<<OCIE1A)|(1<<OCIE1B);	// enable compare-match interrupts

	clearDisplay();
	setBrightness(0);
	setSpacing(1);
	setScrollSpeed(7, 10);
	setScrollMode(FORWARD, 1);
}


void Hacklace::setBrightness(byte br)
{
	// limit range to 0..DISP_MAX_BRIGHT
	if (br > DISP_MAX_BRIGHT)  br = DISP_MAX_BRIGHT;
	dimming = DISP_MAX_DIM >> br;
}


byte Hacklace::getBrightness()
{
	byte br, dim;

	br = 3;
	dim = dimming >> 1;
	while (dim > 0) {
		br--;
		dim >>= 1;
	}
	return(br);
}


void Hacklace::setSpacing(byte spc)
{
	spacing = spc;
}


void Hacklace::setScrollSpeed(byte speed, byte delay)
{
	speed &= 0x0F;		// limit range to 0..15
	delay &= 0x0F;		// limit range to 0..15
	scroll_speed = pgm_read_byte(&spd_conv[speed]);
	scroll_delay = pgm_read_byte(&dly_conv[delay]);
}


void Hacklace::setScrollMode(byte dir, byte inc)
{
	dir &= (SCROLL_BACK|SCROLL_BIDIR|SCROLL_OFF);	// limit range
	inc &= 0x0F;									// limit range
	if (dir & SCROLL_BACK)	{ delay_counter = 0; }
	else					{ delay_counter = scroll_delay; }
	scroll_mode = inc | dir;
}


void Hacklace::setCursor(unsigned int cur)
{
	if (cur < DISP_MAX)	{ cursor = cur; }
	else				{ cursor = DISP_MAX; }
}


void Hacklace::cursorHome()
{
	cursor = 0;
}


void Hacklace::setViewport(unsigned int vp)
{
	if (vp < (DISP_MAX - DISP_COLS))	{ viewport = vp; }
	else								{ viewport = DISP_MAX - DISP_COLS; }
}


void Hacklace::clearDisplay()
{
	byte i;
	
	cursor = 0;
	viewport = 0;
	// instead of clearing the whole display memory, we only clear the current viewport
	for (i = 0; i < DISP_COLS; i++) {
		displayMemory[i] = 0;
	}
}


void Hacklace::printByte(byte byt)
{
	if (cursor < DISP_MAX) {
		displayMemory[cursor] = byt;
		cursor++;
	}
}


void Hacklace::printImage_P(const byte* img)
{
	// print image stored in flash memory
	// first byte = image width (number of bytes to follow)

	unsigned int	width;		// image width
	unsigned int	free;		// number of free bytes in display memory
	
	width = (unsigned int) pgm_read_byte(img++);	// get character width
	free = DISP_MAX - cursor;
	if (width > free) { width = free; }
	memcpy_P(&displayMemory[cursor], img, width);	// copy pixel data to display memory
	cursor += width;
}


void Hacklace::printChar(byte ch)
{
	if (cursor >= DISP_MAX) { return; }						// return if display memory is full
	ch -= CHAR_BASE;
	if ( ch >= ( sizeof(font)/sizeof(font[0]) ) ) { return; }
	printImage_P( (unsigned char*) pgm_read_word(&font[ch]) );	// get pointer to pixel data and 
																// print character
}


const char* Hacklace::printString(const char* st)
{
	// print zero-terminated string stored in RAM
	return( printStr(st, RAM) );
}		


const char* Hacklace::printString_P(const char* st)
{
	// print zero-terminated string stored in flash memory
	return( printStr(st, FLASH) );
}		


const char* Hacklace::printString_EE(const char* st)
{
	// print zero-terminated string stored in EEPROM
	return( printStr(st, EEPROM) );
}		


const char* Hacklace::printStr(const char* st, byte src_mem_type)
{
	// print zero-terminated string
	// Characters are interpreted using the font.
	// Raw mode is entered by RAW_MODE_CHAR followed by number of raw bytes.
	// A pointer to the byte after the terminating zero is returned.
	
	byte ch, i;

	ch = readByte(st++, src_mem_type);
	while (ch) {
		if (ch == RAW_MODE_CHAR) {
			ch = readByte(st++, src_mem_type);		// read number of raw bytes that follow
			for (i=0; i<ch; i++) { printByte(readByte(st++, src_mem_type)); }
		} else {
			printChar(ch);
		}
		// insert spacing after a printable or user defined character
		// but not after fixed spaces or animations
		if ((ch < SPC8) || (ch > SPC1)) {
			for (i=0; i<spacing; i++) { printByte(0); }
		}
		ch = readByte(st++, src_mem_type);
	}
	return(st);
}


byte Hacklace::readByte(const char* ptr, byte src_mem_type)
{
	if (src_mem_type == RAM)		{ return( *ptr ); }					// read byte from RAM
	else if (src_mem_type == FLASH)	{ return( pgm_read_byte(ptr) ); }	// read byte from FLASH
	else { return( eeprom_read_byte((const unsigned char*)ptr) ); }		// read byte from EEPROM
}


void Hacklace::printMiniDigit(byte v, byte y)
{
	// Print a hexadecimal digit using a 3 x 5 font at the current cursor.
	// Former display content is overwritten.
	// y = y-position of the digit
	
	byte		i;
	const byte*	ptr;
	
	v &= 0x0F;									// use only lower nibble of digit
	y &= 0x03;									// limit range to 0..3
	ptr = &minidigits[(v << 1) + v];			// (v << 1) + v = 3 * v
	for (i = 0; i < 3; i++) {
		v = pgm_read_byte(ptr++);
		printByte(v << y);
	}	
}


byte Hacklace::getPixel(word x, byte y)
{
	// return the pixel state (1 = on, 0 = off)
	if ( ((displayMemory[x])) & BV_LOOKUP(y) )
		return(1);
	else
		return(0);
}


void Hacklace::setPixel(word x, byte y, byte pen)
{
	// set pixel state (pen >= 1 --> on; pen = 0 --> off)
	if ( (x >= DISP_MAX) || (y > 7) ) { return; }
	if (pen) {
		displayMemory[x] |= BV_LOOKUP(y);
	} else {
		displayMemory[x] &= ~BV_LOOKUP(y);
	}
}


void Hacklace::drawRect(word x1, byte y1, word x2, byte y2, byte pen)
{
	// Draw a filled rectangle.
	// pen >= 1  turn LEDs on;   pen = 0  turn LEDs off

	word i;
	byte mask;
	
	if (x1 > (DISP_MAX - 1)) { x1 = (DISP_MAX - 1); }
	if (x2 > (DISP_MAX - 1)) { x2 = (DISP_MAX - 1); }
	y1 &= 0x07;									// limit range to 0..7
	y2 &= 0x07;									// limit range to 0..7
	
	mask = 0;
	for (i = y1; i <= y2; i++) {
		mask |= BV_LOOKUP(i);
	}
	for (i = x1; i <= x2; i++) {
		if (pen) {
			displayMemory[i] |= mask;
		} else {
			displayMemory[i] &= ~mask;
		}
	}	
}


void Hacklace::print0_99(byte val, byte y)
{
	// print a value in the range 0..99 using two mini-digits
	
	byte		i, bcd;

	if (val > 99) { val = 99; }

	// convert val to BCD format
	bcd = swap(val) & 0x0F;
	for (i=0; i<4; i++) {
		if ((bcd & 0x0F) > 4) { bcd += 3; }
		bcd <<= 1;  val <<= 1;
		if (val & 0x10) { bcd++; }
	}

	// print digits
	printMiniDigit(swap(bcd) & 0x0F, y);
	printByte(0);  printByte(0);
	printMiniDigit(bcd & 0x0F, y);
}


void Hacklace::scrollDisplay()
{
	// do a single scrolling step
	
	unsigned int	new_viewport;
	byte			mode, inc;

	mode = scroll_mode;
	if (mode & SCROLL_OFF) { return; }
	if (cursor <= DISP_COLS) { return; }	// no scrolling if content of display memory
											// is smaller than dot matrix
											
	inc = mode & 0x0F;						// extract increment
	if (mode & SCROLL_BACK)	{				// scrolling backward
		new_viewport = viewport - inc;		// We use a dirty trick here:
	}										// new_viewport may underflow at left end of display memory.
	else {
		new_viewport = viewport + inc;		// scrolling forward
	}

	if ( new_viewport > (cursor - DISP_COLS) ) {	// end of scrolling range reached?
											// Note: As new_viewport is allowed to underflow, this is 
											// true at both ends of the display memory.
		if (delay_counter) {				// wait until delay has elapsed
			delay_counter--;
		}
		else {
			ATOMIC_BLOCK(ATOMIC_FORCEON) {
				sync_flags |= SCROLL_SYNC;	// set scroll flag
			}
			delay_counter = scroll_delay;			// reload delay counter
			if (mode & SCROLL_BIDIR) {
				scroll_mode = mode ^ SCROLL_BACK;	// reverse direction
			} else if (mode & SCROLL_BACK) {
				viewport = cursor - DISP_COLS;		// restart from right end
			} else {
				viewport = 0;						// restart from left end
			}
		}
	}
	else {
		viewport = new_viewport;
	}
}


void Hacklace::sampleButtons()
{
	byte pb;

	pb = ~(PIN(BTN_PORT)) & btn_mask;			// sample push buttons
	if (pb == 0) {								// --- no button pressed ---
		if (button & PB_PRESS) {				// former state = pressed ?
			if ((button & (PB_ACK|PB_LONG)) == (PB_ACK|PB_LONG)) {	// in case of an acknowledged longpress event
				button &= ~(PB_ACK | PB_PRESS);		// -> issue long release event (clear ack and press)
			} else {
				button &= ~(PB_ACK | PB_LONGPRESS);	// -> issue release event (clear ack, long and press)
			}
		}
	}
	else {										// --- button pressed ---
		if ((button & PB_PRESS) == 0) {			// former state = all buttons released ?
			button = pb | PB_PRESS;				// -> issue press event
			pb_timer = BTN_LONGPRESS_DELAY;		// start push button timer
		} else {
			if ((button & (BTN_MASK|PB_LONG)) == pb) {	// holding same button pressed?
				if (pb_timer == 0) {			// if push button timer has elapsed
					button = pb | PB_LONGPRESS;	// issue longpress event
				}
			}
		}
	}
	if (pb_timer) { pb_timer--; }
}


void Hacklace::buttonAck()
{
	button |= PB_ACK;						// set acknowledge flag
}


void Hacklace::disableButton2()
{
	btn_mask = BUTTON1;
}


void Hacklace::enableButton2()
{
	btn_mask = BTN_MASK;
	DDR(BTN_PORT)  &= ~(BUTTON2);			// switch to Input
	PORT(BTN_PORT) |= (BUTTON2);			// enable pull-up
}


byte Hacklace::sysTimerHasElapsed()
{
	if (sync_flags & SYS_TIMER_SYNC) { return(1); }
	else { return(0); }
}


byte Hacklace::scrollSync()
{
	if (sync_flags & SCROLL_SYNC)	{ 
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
			sync_flags &= ~SCROLL_SYNC;
			return(1);
		}
	}
	return(0);
}


byte Hacklace::columnSync()
{
	if (sync_flags & COLUMN_SYNC)	{ 
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
			sync_flags &= ~COLUMN_SYNC;
			return(1);
		}
	}
	return(0);
}


void Hacklace::disableDisplay()
{
	DDRC = 0;								// disable column outputs
	DDRD &= ~PORTD_COL_MASK;
	TCCR1B = 0;								// stop timer 1
}


void Hacklace::enableDisplay()
{
	DDRC = DDRC_INIT;						// enable column outputs
	DDRD |= PORTD_COL_MASK;
	TCCR1B = (T1_PRESC<<CS10);				// run timer 1
}


void Hacklace::run()
{
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		sync_flags &= ~SYS_TIMER_SYNC;		// clear system timer flag
	}
	
	if (scroll_timer) {
		scroll_timer--;
	}
	else {
		scroll_timer = scroll_speed;		// restart timer
		scrollDisplay();					// do a scrolling step
	}

	sampleButtons();						// sample push buttons
}


void Hacklace::goToSleep()
{
#ifdef ENABLE_RUN_SLEEP_MONITOR
	PORTD &= ~(1<<3);
#endif
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
#ifdef ENABLE_RUN_SLEEP_MONITOR
	PORTD |= (1<<3);
#endif
}


void Hacklace::powerDown()
{
	// turn outputs off; enable pull-ups on input pins to avoid floating inputs
	PORTB = 0xFF;
	PORTC = 0xFF;
	PORTD = 0xFF;
	
	// power down peripheral units
	ADCSRA &= ~(1<<ADEN);					// disable ADC
	PCIFR |= (1<<PCIF2);					// clear interrupt flag
	PCMSK2 |= (1<<PCINT18);					// enable PCINT18
	PCICR |= (1<<PCIE2);					// enable pin change interrupt
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();
	// --- after wakeup ---
	ADCSRA |= (1<<ADEN);					// enable ADC
}


void Hacklace::enableFreqCounter()
{
	overfl_cnt    = 0;
	int1_cnt      = 0;
	interval_accu = 0;
	EICRA = (2 << ISC10);					// generate interrupt on falling edge on PD3
	EIFR  = (1 << INT1);					// clear interrupt flag INT1
	EIMSK = (1 << INT1);					// enable interrupts on PD3 (= INT1)
	TIFR1  |= (1<<TOV1);					// clear timer 1 overflow interrupt flag
	TIMSK1 |= (1<<TOIE1);					// enable timer 1 overflow interrupts	
}


void Hacklace::disableFreqCounter()
{
	EIMSK &= ~(1 << INT1);					// disable interrupts on PD3 (= INT1)
	TIMSK1 &= ~(1<<TOIE1);					// disable timer 1 overflow interrupts	
}


void Hacklace::getFreqCounter(unsigned long* time, word* count)
// Return accumulated time interval and number of interrupts.
{
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		*time = interval_accu;
		interval_accu = 0;
		*count = int1_cnt;
		int1_cnt = 0;
	}
}


float Hacklace::getFrequency()
// Return (average) interrupt frequency (in Hertz).
// Note: Make sure there have been less than 65536 interrupts between two calls of this function.
{
	unsigned long	interval;
	word			cnt;
	
	getFreqCounter(&interval, &cnt);
	if (interval == 0) { return(0); }
	return( 1000000.0 * (float) cnt / ((float) interval) );
}


float Hacklace::getPeriod()
// Return (average) time between two succeeding interrupts (in microseconds).
// Note: Make sure there have been less than 65536 interrupts between two calls of this function.
{
	unsigned long	interval;
	word			cnt;
	
	getFreqCounter(&interval, &cnt);
	if (cnt == 0) { return(0); }
	return( ((float) interval) / (float) cnt );
}


word Hacklace::getInt1Count()
// Return number of int1 interrupts.
// Note: Make sure there have been less than 65536 interrupts between two calls.
// Note: Call enableFreqCounter() before using this function.
{
	word cnt;
	
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		cnt = int1_cnt;
	}
	return( cnt );
}


inline void Hacklace::outputColumn()
{
	byte pixels;		// pixel content of current column
	byte col_bit;		// bit pattern in which the bit for the current column is set
	
	// all columns off
	PORTC = PORTC_INIT;
	PORTD |= (PORTD_COL_MASK | PORTD_ROW_MASK);

	current_column++;
	if (current_column >= (DISP_COLS * dimming)) { current_column = 0; }
	if (current_column >= DISP_COLS) return;
	col_bit = BV_LOOKUP(current_column);
	pixels = displayMemory[viewport + current_column];
	
	// output pixels of current column on display rows
	PORTB = ~pixels;
	PORTD &= ~(pixels & PORTD_ROW_MASK);
	// turn current column on
	PORTC = ~col_bit;
	PORTD &= ~((col_bit>>2) & PORTD_COL_MASK);
	sync_flags |= COLUMN_SYNC;
}


inline void Hacklace::setSysTimerFlag()
{
	sync_flags |= SYS_TIMER_SYNC;
}


inline void Hacklace::int1Handler()
{
	ulong_hilo	temp;
	word		tcnt_new;

/*
	tcnt_new = TCNT1;
	interval = tcnt_new - tcnt_old;
	interval_accu += interval;
	int1_cnt++;
	tcnt_old = tcnt_new;
*/
	tcnt_new = TCNT1;
	temp.u16hi = overfl_cnt;
	overfl_cnt = 0;
	temp.u16lo = tcnt_new;
	temp.u32 -= (unsigned long) tcnt_old;
	interval_accu += temp.u32;
	tcnt_old = tcnt_new;
	int1_cnt++;
}


inline void Hacklace::t1overflowHandler()
{
	if (overfl_cnt < 0xFFFF) {
		overfl_cnt++;
	}
}


/**********************
 * interrupt routines *
 **********************/

ISR(TIMER1_COMPA_vect)
// column interrupt
{	
	OCR1A += OCR1A_CYCLE_TIME;				// setup next interrupt cycle
	sei();
	Hacklace::outputColumn();				// show column on dot matrix display
}


ISR(TIMER1_COMPB_vect)
// system timer interrupt
{	
	OCR1B += OCR1B_CYCLE_TIME;				// setup next interrupt cycle
	Hacklace::setSysTimerFlag();
}


ISR(PCINT2_vect)
// pin change interrupt (for wake-up)
{
	PCICR &= ~(1<<PCIE2);					// disable pin change interrupt
}


ISR(INT1_vect)
// edge on PD3
{
	Hacklace::int1Handler();
}


ISR(TIMER1_OVF_vect)
// timer 1 overflow
{
	Hacklace::t1overflowHandler();
}
