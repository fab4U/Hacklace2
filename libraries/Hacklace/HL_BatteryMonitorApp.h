/*
 * HL_BatteryMonitorApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Battery Monitor App

					Use ADC6 to measure the voltage of the lithium battery. Display 
					a battery symbol which is filled accordingly.
					
					Make a voltage divider out of two resistors. Connect first resistor 
					(R_UPPER) between ADC6 and VCC and second resistor (R_LOWER) 
					between ADC6 and PD3. Suggested values are
					R_UPPER = 22 kOhms
					R_LOWER = 10 kOhms
					Enter resistor values below.
					
					Set UPDATE_INTERVAL to define how frequently the battery is checked.
										
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

#define APP_NAME		BatteryMonitorApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

// update interval
#define UPDATE_INTERVAL	1.0		// check battery every UPDATE_INTERVAL seconds (range 0.1s - 2.5s)

// voltage divider
#define R_UPPER			22.00	// value of upper resistor (in kOhms)
#define R_LOWER			10.00	// value of lower resistor (in kOhms)
#define R13				1000.00	// value of resistor R13 (in kOhms)

// internal reference
#define V_REFERENCE		1.10	// nominal value of internal reference voltage (see ATMEL datasheet)

// factor for converting adc-value to millivolts
#define ADC_FACTOR		( (R_UPPER + R_LOWER) / R_LOWER * (V_REFERENCE / 1.023) )

// voltage thresholds (in millivolts)
const int VoltThresholds[5] PROGMEM =	{2800, 2700, 2580, 2420, 2200};

// graphical image of a battery
const byte BatteryIcon[] PROGMEM =	{8, 0x00, 0x00, 0xFE, 0x83, 0x83, 0xFE, 0x00, 0x00};


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
		static byte timer;
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::timer;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	HL.setScrollMode(NO_SCROLLING, 1);
	analogReference(INTERNAL);
	timer = 0;
	pinMode(3, OUTPUT);
	return( ee );
}


void APP_CLASSNAME::run()
{
	word	ana;							// adc reading
	word	volt;							// voltage in millivolts
	byte    h;
	
	if (timer > 0) { timer--;  return; }
	timer = (byte)(0.5 + UPDATE_INTERVAL * 100) - 1;

											// to avoid disturbances of the a/d conversion
	HL.disableDisplay();					// turn display off
	digitalWrite(3, LOW);					// turn voltage divider on
	delay(1);								// wait 1 ms for voltage to stabilize
	ana = analogRead(A6);
	digitalWrite(3, HIGH);					// turn voltage divider off
	HL.enableDisplay();						// turn display on again
	volt = (word) (0.5 + ADC_FACTOR * (float) ana);
	
	for (h = 0; h < 5; h++) {
		if ( volt > pgm_read_word(&VoltThresholds[h]) ) break;
	}
	HL.cursorHome();
	HL.printImage_P(BatteryIcon);
	HL.drawRect(3, h+2, 4, 7, 1);
}


void APP_CLASSNAME::finish()
{
	pinMode(3, INPUT);
}


