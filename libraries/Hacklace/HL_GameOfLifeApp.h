/*
 * HL_GameOfLifeApp.cpp
 *
 */ 

/**********************************************************************************

Description:		Hacklace Game-Of-Life App

					Implements Conways game of life.
					Reads 10 parameter bytes from EEPROM.
					Parameter 1 gives the number of iterations.
					Parameter 2 specifies the delay between two updates.
					Parameters 3..10 define the start configuration.

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

#define APP_NAME		GameOfLifeApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)

#define R_MAX			7


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
		static byte viewport;
		static byte iteration;
		static byte max_iterations;
		static byte life_cycle_delay;
		static const unsigned char* start_config_ptr;
		
		void setCell(byte x, byte y, byte state);
		void spawnCell(byte x, byte y);
		void killCell(byte x, byte y);
		byte getCell(byte x, byte y);
		byte sumNeighbours(byte x, byte y);
};


APP_CLASSNAME APP_NAME;		// create an instance of the app class


/**************************
 * static class variables *
 **************************/

byte 	APP_CLASSNAME::timer;
byte 	APP_CLASSNAME::viewport;
byte 	APP_CLASSNAME::iteration;
byte 	APP_CLASSNAME::max_iterations;
byte 	APP_CLASSNAME::life_cycle_delay;
const unsigned char* APP_CLASSNAME::start_config_ptr;


/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
	HL.setScrollMode(NO_SCROLLING, 1);
	HL.clearDisplay();
	viewport = 0;
	// load number of iterations
	max_iterations = eeprom_read_byte(ee++) - 1;
	iteration = 0;
	// load delay between updates
	life_cycle_delay = eeprom_read_byte(ee++) - 1;
	timer = 0;
	// load start configuration
	start_config_ptr = ee;
	eeprom_read_block(&(HL.displayMemory[0]), start_config_ptr, 8);	// load start configuration
	return( ee + 8 );
}


void APP_CLASSNAME::run()
{
	byte i, j, sum_of_neighbours;
	
	if (HL.button == BTN2_RELEASED) {
		HL.buttonAck();
		iteration = max_iterations;
		eeprom_read_block(&HL.displayMemory[viewport], start_config_ptr, 8);	// load start configuration	
		timer = life_cycle_delay;
		return;
	}
	
	if (timer > 0) { timer--;  return; }
	timer = life_cycle_delay;

	if (iteration > 0) {
		iteration--;
		
		// calculate next frame
		for (i=0; i<8; i++) {
			for (j=0; j<8; j++) {
				sum_of_neighbours = sumNeighbours(i, j);
				if (sum_of_neighbours == 2) {
					setCell(i, j, getCell(i, j));			// cell remains unchanged
				} 
				else if (sum_of_neighbours == 3) {
					setCell(i, j, 1);						// new cell is born
				}
				else { 
					setCell(i, j, 0);						// cell dies
				}
			}
		}
		viewport ^= 8;				// switch viewport
		HL.setViewport(viewport);
	}
	else {
		iteration = max_iterations;
		eeprom_read_block(&HL.displayMemory[viewport], start_config_ptr, 8);	// load start configuration	
	}
}


void APP_CLASSNAME::finish()
{
}


byte APP_CLASSNAME::getCell(byte x, byte y)
{
	// return the cell state (1 = alive, 0 = empty/dead)
	x &= 0x07;  y &= 0x07;			// wrap around at the edges
	return ( HL.getPixel(viewport + x, y) );
}


void APP_CLASSNAME::setCell(byte x, byte y, byte state)
{
	// set cell state

	int vp;
	
	x &= 0x07;  y &= 0x07;			// wrap around at the edges
	vp = viewport ^ 8;				// write to new viewport
	HL.setPixel(vp + x, y, state);
}


byte APP_CLASSNAME::sumNeighbours(byte x, byte y)
{
	// return the sum of the cell states of all 8 adjacent cells
	
	byte i, sum;
	
	// start with left and right neighbours
	sum = getCell(x-1, y) + getCell(x+1, y);
	// add top and bottom neighbours
	x--;
	for (i=0; i<3; i++) {
		sum += ( getCell(x, y-1) + getCell(x, y+1) );
		x++;
	}
	return(sum);
}

