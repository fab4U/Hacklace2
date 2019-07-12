/*
 * Hacklace_AppEngine.h
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


#ifndef HACKLACE_APPENGINE_H_
#define HACKLACE_APPENGINE_H_

#include "Arduino.h"
#include "Hacklace_App.h"


/*************
 * constants *
 *************/

// version
#define HL_VERSION_MAJOR	0
#define HL_VERSION_MINOR	7

// start and end address of text/animation data in EEPROM
#define EE_START_ADDR		0x000
#define EE_END_ADDR			0x3FF

// EEPROM address of parameters for reset app
#define EE_RESET_PARAMS		0x001

// app IDs
#define ANIMATION			0
#define SPIRAL				1
#define COUNTER				2
#define GAME_OF_LIFE		3
#define VOLTMETER			4
#define BATT_MONITOR		5
#define FREQMETER			6
#define THERMOMETER			7
#define STOPWATCH			8
#define SPEEDOMETER			9
#define JUKEBOX				12
#define FOUR_WINS			13
#define SNAKE				14
#define JUMP				15
#define FROGGER				16
#define TICTACTOE			17
#define CODE_LOCK			20
#define DISPLAY_TEST		21
#define EXAMPLE_APP			30
#define RESET_APP			31

// special App-ID (do not change)
#define END_OF_LIST			255			// app_id value that indicates the end of the app list

// select animation by index (starting from 0)
#define ANI(index)			(128 + index)

// maximum number of apps
#define MAX_APPS			32

// default EEPROM content (see "Hacklace_AppEngine.cpp")
extern const byte ee_default[];
extern const unsigned int ee_default_size;


/***********
 * classes *
 ***********/

class Hacklace_AppEngine : public Hacklace
{
	public:
		static void initialize();
		static void copyToEeprom(const byte* flash_ptr, unsigned int size);
		static Hacklace_App* getApp(byte app_id);
		static void resetApp();
		static void nextApp();
		static void run();

	private:
		static const unsigned char*	ee_ptr;
		static Hacklace_App*		app;					// pointer to current app
		
		static void enterPowerDown();
};


extern Hacklace_AppEngine HL;


#endif /* HACKLACE_APPENGINE_H_ */
