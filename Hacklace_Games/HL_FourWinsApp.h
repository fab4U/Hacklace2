/**********************************************************************************

Description:		Four Wins App

					Try to get four chips in a row (horizontally, vertically or 
					diagonal).
					
					Use right button to control the game.
					
					First select one or two players. Hold right button pressed to 
					choose selection.
					Press right button to move through columns. Hold right button 
					pressed to drop chip.
					Computer chips are the blinking ones.

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
#define APP_NAME		FourWinsApp
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
                byte showP2;
                byte markerPos;
                byte playerMove; 
                boolean showGameMenu;
                boolean buttonPressed;
                boolean buttonLongPressed;
                boolean newMove;
                boolean winnerDisplay;          
                boolean gameMapP1[8][8];
                boolean gameMapP2[8][8];
                int menuPos;
                boolean KI;
                
                void showMap();
                void doMove();
                void moveMarker();      
                void initGame();
                int winner();
                void displayMenu(); 
                void doKIMove();
                void undoMove();          
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

void APP_CLASSNAME::displayMenu()
{
  HL.clearDisplay();
  if(menuPos==1)
    HL.printString("1P");
  else
    HL.printString("2P");
}

void APP_CLASSNAME::run()
{
    if(showGameMenu)
    {
      displayMenu();
      
       if(HL.button == BTN2_PRESSED)
          buttonPressed=true;
       if(HL.button == BTN2_LONGPRESSED)
          buttonLongPressed=true;
       if(HL.button == BTN2_RELEASED&&buttonLongPressed)
       {
           showGameMenu=false;
           buttonLongPressed=false;
           buttonPressed=false;
           if(menuPos==1)
              KI = true;
           else
              KI = false;
           HL.clearDisplay();
       }
       if(HL.button == BTN2_RELEASED&&buttonPressed)
       {
          buttonPressed=false;
          if(menuPos==1)
             menuPos=2;
          else
             menuPos=1;
       }
    }
    else
    { 
       if(winner()!=0)
       {
         if(!winnerDisplay)
         {
           winnerDisplay=true;
           HL.setScrollSpeed(7, 7);
  	   HL.setScrollMode(FORWARD, 1);
           if(winner()==1)
              HL.printString("P1 Wins");
           else
              HL.printString("P2 Wins");
         } 
    
         if(HL.button == BTN2_LONGPRESSED)
              buttonLongPressed=true;
         if(HL.button == BTN2_RELEASED&&buttonLongPressed)
              initGame();   
       }
       else
       {
         if(newMove)
            {
              if(KI&&playerMove==2)
              {
                  doKIMove();
              }
              else
              {             
                  newMove=false;
                  markerPos=0;
                  if(playerMove==1)
                    gameMapP1[0][0]=true;
                  else
                    gameMapP2[0][0]=true;
              }
            }
            else
            {  
              if(HL.button == BTN2_RELEASED && buttonLongPressed)
              {
                doMove();
                buttonPressed=false;
                buttonLongPressed=false;
                newMove=true;
              }
              
              if(HL.button == BTN2_RELEASED&&buttonPressed)
              {
                moveMarker();
                buttonPressed=false;
              }
      
              if(HL.button == BTN2_LONGPRESSED)
                  buttonLongPressed=true;         
                  
              if(HL.button == BTN2_PRESSED)
                  buttonPressed=true;
            }
            
            if(showP2==3)
               showP2=0;
            else
               showP2++;  
            showMap();
       }
    }
}

int APP_CLASSNAME::winner()
{
  int winner=0;
  
   for(int i=0;i<5;i++)    // Prüfe Horizontal
   {
      for(int j=1;j<8;j++)
      {
          if(gameMapP1[i][j]&&gameMapP1[i+1][j]&&gameMapP1[i+2][j]&&gameMapP1[i+3][j])
                 winner=1;
          if(gameMapP2[i][j]&&gameMapP2[i+1][j]&&gameMapP2[i+2][j]&&gameMapP2[i+3][j])
                 winner=2;
      }
   }
   
   for(int i=0;i<8;i++)  // Prüfe Vertikal
   {
      for(int j=1;j<5;j++)
      {
          if(gameMapP1[i][j]&&gameMapP1[i][j+1]&&gameMapP1[i][j+2]&&gameMapP1[i][j+3])
                 winner=1;
          if(gameMapP2[i][j]&&gameMapP2[i][j+1]&&gameMapP2[i][j+2]&&gameMapP2[i][j+3])
                 winner=2;
      }
   }
   
   for(int i=0;i<8;i++)  // Prüfe Diagonal
   {
      for(int j=1;j<5;j++)
      {
          if(i<5)
          {
            if(gameMapP1[i][j]&&gameMapP1[i+1][j+1]&&gameMapP1[i+2][j+2]&&gameMapP1[i+3][j+3])
                 winner=1;
            if(gameMapP2[i][j]&&gameMapP2[i+1][j+1]&&gameMapP2[i+2][j+2]&&gameMapP2[i+3][j+3])
                 winner=2;
          }
          if(i>2)
          {
            if(gameMapP1[i][j]&&gameMapP1[i-1][j+1]&&gameMapP1[i-2][j+2]&&gameMapP1[i-3][j+3])
                 winner=1;
            if(gameMapP2[i][j]&&gameMapP2[i-1][j+1]&&gameMapP2[i-2][j+2]&&gameMapP2[i-3][j+3])
                 winner=2;               
          }
      }
   }
         
   return winner;         
}

void APP_CLASSNAME::initGame()
{
  for(int i=0;i<8;i++)
  {
    for(int j=0;j<8;j++)
    {
      gameMapP1[i][j]=false;
      gameMapP2[i][j]=false;
    }
  }
  
  showP2 = 0;
  playerMove = 1;
  buttonPressed=false;
  buttonLongPressed=false;
  newMove = true;
  markerPos=0;
  winnerDisplay=false;
  showGameMenu=true;
  menuPos=1;
  KI = true;
}

void APP_CLASSNAME::moveMarker()
{
  if(playerMove==1)
  {
    gameMapP1[markerPos][0]=false;
    if(markerPos==7)
       markerPos=0;
    else
       markerPos++;
    gameMapP1[markerPos][0]=true;
  }
  
  if(playerMove==2)
  {
    gameMapP2[markerPos][0]=false;
    if(markerPos==7)
       markerPos=0;
    else
       markerPos++;
    gameMapP2[markerPos][0]=true;
  }
}
 
void APP_CLASSNAME::doMove()
{
  if(playerMove==1)
  {
    gameMapP1[markerPos][0]=false;
    if(gameMapP1[markerPos][1]||gameMapP2[markerPos][1])
       playerMove=1;
    else
       playerMove=2;
    
    for(int i=7;i>0;i--)
    {
      if(gameMapP1[markerPos][i]==false&&gameMapP2[markerPos][i]==false)
      {
        gameMapP1[markerPos][i]=true;
        break;
      }
    }
  }
  else
  {
    gameMapP2[markerPos][0]=false;
    if(gameMapP1[markerPos][1]||gameMapP2[markerPos][1])
       playerMove=2;
    else
       playerMove=1;
    for(int i=7;i>0;i--)
    {
      if(gameMapP2[markerPos][i]==false&&gameMapP1[markerPos][i]==false)
      {
        gameMapP2[markerPos][i]=true;
        break;
      }
    }
  }
}
     
void APP_CLASSNAME::showMap()
{
  for(int i=0;i<8;i++)
  {
    for(int j=0;j<8;j++)
    {
      if(gameMapP1[i][j])
        HL.setPixel(i,j,1);
     else
        HL.setPixel(i,j,0);
     
     if(gameMapP2[i][j]&&showP2==0)
        HL.setPixel(i,j,1);
    }
  }
}

void APP_CLASSNAME::undoMove()
{
    for(int i=7;i>0;i--)
    {
      if(!gameMapP1[markerPos][i]&&!gameMapP2[markerPos][i])
      {
        gameMapP1[markerPos][i+1]=false;
        gameMapP2[markerPos][i+1]=false;
        break;
      }
    }
}

void APP_CLASSNAME::doKIMove()
{
    boolean gotGoodMove=false;
    for(int i=0;i<8;i++)
    {
      markerPos=i;              
      playerMove=2;
      doMove();
      if(winner()!=0&&playerMove==1)
      {
        gotGoodMove=true;
        undoMove();
        break;
      } 
      if(playerMove==1)
        undoMove();
    }
    
    if(!gotGoodMove)
    {
        for(int i=0;i<8;i++)
        {  
          markerPos=i;  
          playerMove=1;
          doMove();
          if(winner()!=0&&playerMove==2)
          {
            gotGoodMove=true;
            undoMove();
            break;
          }
          if(playerMove==2)
            undoMove();
        }
    }
    
    if(!gotGoodMove)
    {
      playerMove=2;
      randomSeed(millis());
      while(playerMove==2)
      {
        markerPos=random(0,8);
        doMove();   
      }
      undoMove();
    }
    
    playerMove=2;
    doMove();
}

void APP_CLASSNAME::finish()
{

}


