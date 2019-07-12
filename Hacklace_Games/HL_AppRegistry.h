/*
 * HL_AppRegistry.h
 *
 */ 

/**********************************************************************************

Author:		Frank Andre
Copyright 2013:	Frank Andre
License:	see "license.md"
Disclaimer:	This software is provided by the copyright holder "as is" and any 
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


#ifndef APP_REGISTRY_H_
#define APP_REGISTRY_H_

#include <avr/pgmspace.h>

#include "Hacklace_App.h"

// include Hacklace apps
#include "HL_AnimationApp.h"
#include "HL_SpiralApp.h"
#include "HL_CounterApp.h"
#include "HL_GameOfLifeApp.h"
//#include "HL_VoltmeterApp.h"
//#include "HL_BatteryMonitorApp.h"
//#include "HL_FreqmeterApp.h"
#include "HL_CodeLockApp.h"
//#include "HL_ThermometerApp.h"
//#include "HL_StopwatchApp.h"
//#include "HL_JukeboxApp.h"
//#include "HL_DisplayApp.h"
#include "HL_FourWinsApp.h"
#include "HL_SnakeApp.h"
#include "HL_JumpApp.h"
#include "HL_FroggerApp.h"
#include "HL_TicTacToeApp.h"
#include "HL_DownloadApp.h"
#include "HL_DisplayTestApp.h"

// make the app_registry globally available
extern const Hacklace_App* const app_registry[MAX_APPS];

// list of all apps
const Hacklace_App* const app_registry[MAX_APPS] PROGMEM = {
	&AnimationApp,			// app-ID 0
	&SpiralApp,			// app-ID 1
	&CounterApp,			// app-ID 2
	&GameOfLifeApp,			// app-ID 3
	&AnimationApp,			// app-ID 4
	&AnimationApp,    		// app-ID 5
	&AnimationApp,			// app-ID 6
	&AnimationApp,	                // app-ID 7
	&AnimationApp,			// app-ID 8
	&AnimationApp,                  // app-ID 9
	&AnimationApp,			// app-ID 10
	&AnimationApp,			// app-ID 11
	&AnimationApp,			// app-ID 12
	&FourWinsApp,			// app-ID 13
	&SnakeApp,			// app-ID 14
	&JumpApp,			// app-ID 15
	&FroggerApp,			// app-ID 16
	&TicTacToeApp,			// app-ID 17
	&AnimationApp,			// app-ID 18
	&AnimationApp,			// app-ID 19
	&CodeLockApp,		        // app-ID 20
	&DisplayTestApp,		// app-ID 21
	&AnimationApp,			// app-ID 22
	&AnimationApp,			// app-ID 23
	&AnimationApp,			// app-ID 24
	&AnimationApp,			// app-ID 25
	&AnimationApp,			// app-ID 26
	&AnimationApp,			// app-ID 27
	&AnimationApp,			// app-ID 28
	&AnimationApp,			// app-ID 29
	&AnimationApp,			// app-ID 30
//	&DisplayApp			// app-ID 31 ResetApp (using HL as serial display)
//	&DisplayTestApp			// app-ID 31 ResetApp (testing the dot-matrix display)
	&DownloadApp			// app-ID 31 ResetApp
					// This app is called after a system reset (NULL -> no app is called)
};

#endif /* APP_REGISTRY_H_ */
