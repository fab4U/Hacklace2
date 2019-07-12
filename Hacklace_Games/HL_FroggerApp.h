/**********************************************************************************

Description:		Frogger App

					You are the little froggy at the bottom end of the display. Try 
					to cross the multi lane highway without getting caught by a car.
					
					Use right button to move one step forward.
					In case of an accident hold right button pressed to restart game.

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
#define APP_NAME	FroggerApp
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
        int lines[6][2];
        boolean gameMap[8][6];
        int posPlayer;
        int points;
        int movement[6];
        boolean startGame;
        boolean buttonPressed;
        boolean scoreDisplay; 
        boolean collision();
        void initGame();
        void showMap();
        void moveParts();
        void initNewLevel(); 
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
    posPlayer=7;
    points=0;
    startGame=false;;
    buttonPressed=false;
    scoreDisplay=false; 
    initNewLevel();
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
       moveParts();
       
       if(HL.button==BTN2_PRESSED)
           buttonPressed=true;
       if(HL.button==BTN2_RELEASED&&buttonPressed)
       {
           posPlayer--;
           buttonPressed=false;
           if(posPlayer==0)
           {
              points++;
              posPlayer=7;
              initNewLevel();
           }
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
  if(posPlayer<7&&posPlayer>0)
  {
    if(gameMap[4][posPlayer-1])
        return true;
  }

  return false;
}

void APP_CLASSNAME::initNewLevel()
{

  randomSeed(millis());
  
  for(int i=0;i<6;i++)
  {
     movement[i]=0;
     int s = random(1,5);
     if(s==1)
     {
        lines[i][0]=2;
        lines[i][1]=30;
     }
     if(s==2)
     {
        lines[i][0]=4;
        lines[i][1]=20;
     }
     if(s==3)
     {
        lines[i][0]=6;
        lines[i][1]=10;
     }
     if(s==4)
     {
        lines[i][0]=8;
        lines[i][1]=4;
     }         
  }

  for(int i=0;i<8;i++)
     for(int j=0;j<6;j++)
        gameMap[i][j]=false; 
}

void APP_CLASSNAME::showMap()
{
  HL.clearDisplay();
  
  for(int i=0;i<8;i++)
  {
    for(int j=0;j<6;j++)
    {
       if(gameMap[i][j])
          HL.setPixel(i,j+1,1);
    }
  }
  
  HL.setPixel(4,posPlayer,1);
}

void APP_CLASSNAME::moveParts()
{
  for(int i=0;i<6;i++)
  {
    if((ticker%(lines[i][0]*10))==0)
    {
       for(int j=1;j<8;j++)
       {
         gameMap[j-1][i]=gameMap[j][i];
       }
       if(gameMap[6][i]&&!gameMap[5][i])
         gameMap[7][i]=true;
       else
         gameMap[7][i]=false;
       
        if((movement[i]%lines[i][1])==0)
           gameMap[7][i]=true;
        movement[i]++;
    }
  }  
}

void APP_CLASSNAME::finish()
{

}

