/**********************************************************************************

Description:		Jump App

					You are kind of Indiana Jones. Jump over the rolling rocks.
					
					Press right button to jump. Hold right button pressed to 
					restart game.

Author:				Students of the Karl Kuebel School in Bensheim, Germany
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

// ##### enter your app name here #####
#define APP_NAME	JumpApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)	// do not change

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
        int ticker; 
        void initGame();
        void showMap();
        boolean collision();
        int partPosition;
        int jumpPhase; 
        int moveSpeed;
        boolean startGame;
        int points;
        boolean scoreDisplay;
        boolean buttonPressed;   
};

APP_CLASSNAME APP_NAME;		// create an instance of the app class

/**************************
 * static class variables *
 **************************/
// Note:  Each static variable name must be prefixed with "APP_CLASSNAME::"

// ##### list your static variables here... #####

/***********
 * methods *
 ***********/

const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
//    HL.setBrightness(DISP_MAX_BRIGHT);
    initGame();
    return( ee );
}

void APP_CLASSNAME::initGame()
{
  ticker=0;
  jumpPhase=0;
  points=0;
  buttonPressed=false;
  partPosition=8;
  moveSpeed=15;
  startGame=false;
  scoreDisplay=false;
}

void APP_CLASSNAME::showMap()
{
  HL.clearDisplay();

  if(ticker%3==0)
  {
    if(jumpPhase==0||jumpPhase==4)
    {
       HL.setPixel(1,4,1);
       HL.setPixel(1,3,1);      
    }
    if(jumpPhase==1||jumpPhase==3)
    {
       HL.setPixel(1,3,1);
       HL.setPixel(1,2,1);     
    }
    if(jumpPhase==2)
    {
       HL.setPixel(1,2,1);
       HL.setPixel(1,1,1);       
    }
  }
  
  if(partPosition!=8)
  {
    HL.setPixel(partPosition,4,1);
  }
     
  for(int i=0;i<8;i++)
    for(int j=5;j<8;j++)
       HL.setPixel(i,j,1);
}


void APP_CLASSNAME::run()
{
   if(!startGame)
   {
     if(HL.button==BTN2_PRESSED)
        buttonPressed=true;
     if(HL.button==BTN2_RELEASED&&buttonPressed)
     {
        startGame=true;
        buttonPressed=false;
     }
     showMap();
   }
   else
   {
       if(!collision())
       {
           ticker++;
           if(ticker%moveSpeed==0)
           {
               if(jumpPhase>0)
                 jumpPhase--;
               if(partPosition>0)
               {
                  partPosition--;
               }
               else
               {
                  partPosition=8;
                  points++;
                  if(points%5==0)
                     moveSpeed--;
               }
           }
              
           if(HL.button==BTN2_PRESSED)
               buttonPressed=true;
               
           if(HL.button==BTN2_RELEASED&&buttonPressed)
           {
             buttonPressed=false;
             jumpPhase=4;
           }  
           showMap();
       } 
     else
     {
        if(!scoreDisplay)
        {
            HL.clearDisplay();   
            scoreDisplay=true;
            HL.setScrollSpeed(7, 7);
            HL.setScrollMode(FORWARD, 1);
            HL.printString("Punkte: ");
            char* text;
            char test[4];
            text=test;
            itoa(points,text,10);
            HL.printString(text);
        }
        else
        {
           if(HL.button == BTN2_LONGPRESSED)
                buttonPressed=true;
           if(HL.button == BTN2_RELEASED&&buttonPressed)
                initGame(); 
        }
     }
   } 
}

boolean APP_CLASSNAME::collision()
{
  if(partPosition==1)
  {
    if(jumpPhase==4||jumpPhase==0||jumpPhase==3)
          return true;
  }
  return false;
}

void APP_CLASSNAME::finish()
{

}

