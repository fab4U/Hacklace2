/**********************************************************************************

Description:		Snake App

					Eat as many of the blinking meat balls as possible but don't eat 
					yourself (i. e. cross your way). With every snack the snake grows 
					longer.
					
					Use right button to make a right turn.
	
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
#define APP_NAME	SnakeApp
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
        int length;
        int snakePositions[64][2];
        int snakeDirection; 
        boolean buttonPressed;
        int ticker;
        boolean showPoint;
        int positionNextPoint[2];
        boolean collision;
        int points;
        boolean scoreDisplay;
 
        void initGame();
        void showMap();
        void moveSnake();
        void checkPoint(); 
        void checkCollision();      
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
  snakePositions[0][0]=1;
  snakePositions[0][1]=1;
  positionNextPoint[0]=5;
  positionNextPoint[1]=6;
  snakeDirection=1;
  length=1;
  buttonPressed=false;
  collision=false;
  ticker=0;
  showPoint=true;
  points=0;
  scoreDisplay=false;
}

void APP_CLASSNAME::showMap()
{
  HL.clearDisplay();
  for(int i=0;i<length;i++)
  {
    HL.setPixel(snakePositions[i][0],snakePositions[i][1],1);
  }
  if(showPoint)
  {
    HL.setPixel(positionNextPoint[0],positionNextPoint[1],1);
    showPoint=false;
  }
  else
  {
    showPoint=true;
  }
}

void APP_CLASSNAME::checkCollision()
{
  for(int i=length-1;i>0;i--)
  {
    if(snakePositions[i][0]==snakePositions[0][0]&&snakePositions[i][1]==snakePositions[0][1])
    {
        collision=true;
    }
  }
} 

void APP_CLASSNAME::run()
{
  if(!collision)
  {
      ticker++;
      if(HL.button == BTN2_PRESSED)
        buttonPressed=true;
      if(HL.button == BTN2_RELEASED && buttonPressed)
      {
        snakeDirection = (snakeDirection+1)%4;
        buttonPressed=false;
      }
      
      if(ticker==20)
      {
        ticker=0;
        moveSnake();
        checkCollision();
        checkPoint();
        showMap();
      }
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

void APP_CLASSNAME::checkPoint()
{
  boolean coll;
  if(snakePositions[0][0]==positionNextPoint[0]&&snakePositions[0][1]==positionNextPoint[1])
  {
    randomSeed(millis());
    do
    {
      coll=false;
      positionNextPoint[0]=random(0,8);
      positionNextPoint[1]=random(0,8);
      for(int i=length-1;i>=0;i--)
      {
        if(snakePositions[i][0]==positionNextPoint[0]&&snakePositions[i][1]==positionNextPoint[1])
        {
          coll=true;
        }
      }
    }while(coll);
    length++;
    points++;
    for(int i=length-1;i>0;i--)
    {
      snakePositions[i][0]=snakePositions[i-1][0];
      snakePositions[i][1]=snakePositions[i-1][1];
    }
  }
}

void APP_CLASSNAME::finish()
{

}

void APP_CLASSNAME::moveSnake()
{
  if(length!=1)
  {
    for(int i=length-1;i>0;i--)
    {
      snakePositions[i][0]=snakePositions[i-1][0];
      snakePositions[i][1]=snakePositions[i-1][1];
    }
  }
  if(snakeDirection==0)
  {
    if(snakePositions[0][1]==0)
       snakePositions[0][1]=7;
    else
       snakePositions[0][1]--;
  }
  if(snakeDirection==1)
  {
    if(snakePositions[0][0]==7)
       snakePositions[0][0]=0;
    else
       snakePositions[0][0]++;
  }
  if(snakeDirection==2)
  {
    if(snakePositions[0][1]==7)
       snakePositions[0][1]=0;
    else
       snakePositions[0][1]++;
  }
  if(snakeDirection==3)
  {
    if(snakePositions[0][0]==0)
       snakePositions[0][0]=7;
    else
       snakePositions[0][0]--;
  }
}

