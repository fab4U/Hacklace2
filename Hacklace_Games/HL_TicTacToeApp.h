/**********************************************************************************

Description:		TicTacToe App

					Try to get three in a row (horizontally, vertically or diagonal).
					
					Use right button to switch field, hold it pressed to occupy field.
					Computer tokens are blinking.

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
#ifdef APP_NAME
	#undef APP_NAME
	#undef APP_CLASSNAME
#endif
#define APP_NAME		TicTacToeApp
#define APP_CLASSNAME	CONCAT(APP_NAME, _class)	// do not change

class APP_CLASSNAME : public Hacklace_App
{
	public:
		const unsigned char* setup(const unsigned char* ee);
		void run();
		void finish();

	private:
                void printMap();
                int currentField;
                int ticker;
                void printField(int);
                boolean buttonPress;
                boolean buttonLongPress;
                boolean isEmpty(int);
                int check();
                void goNext();
                int fields[9];
                void choose(int,int);
                int randomField();
                int r;
                boolean showResult;
                void initGame();
};
APP_CLASSNAME APP_NAME;
const unsigned char* APP_CLASSNAME::setup(const unsigned char* ee)
{
//        HL.setBrightness(3);
        currentField = 0;
        for(int i=0;i<9;i++)
            fields[i]=0;
        showResult=false;
	return( ee );
}

void APP_CLASSNAME::initGame()
{
        currentField = 0;
        for(int i=0;i<9;i++)
            fields[i]=0;
        showResult=false;
        buttonPress=false;
        buttonLongPress=false;
        ticker=0;     
}

void APP_CLASSNAME::run()
{
  if(check()==0)
  {
      if(ticker==30)
        ticker=0;   
      else
        ticker++;
      if(HL.button==BTN2_PRESSED)
        buttonPress = true;
      if(HL.button==BTN2_LONGPRESSED)
      {
        buttonLongPress = true;
        buttonPress = false;
      }
      if(HL.button==BTN2_RELEASED&&buttonPress)
      {
        buttonPress = false;
        goNext();
      }
      if(HL.button==BTN2_RELEASED&&buttonLongPress)
      {
        buttonLongPress = false;
        choose(currentField, 1);
        goNext();
        if(check()==0)
          choose(randomField(), 2);
      }
      printMap();
  }
  else
  {
    if(!showResult)
    {
        showResult=true;
        HL.setScrollSpeed(7,7);
        HL.setScrollMode(FORWARD,1);
        if(check()==1)
        {
          HL.printString("P1 Wins!");
        }
        if(check()==2)
        {
          HL.printString("P2 Wins!");
        }
        if(check()==3)
        {
          HL.printString("Draw!");
        }
    }
    else
    {
        if(HL.button==BTN2_PRESSED)
             buttonPress=true;
        if(HL.button==BTN2_RELEASED&&buttonPress)
             initGame();
    }
  }
}

void APP_CLASSNAME::goNext()
{
   do
   {
     currentField++;
     if(currentField==9)
     {
       currentField=0;
     }  
   }
   while(!isEmpty(currentField));
}

boolean APP_CLASSNAME::isEmpty(int n)
{
  if(fields[n] == 0) 
    return true;
  else
    return false;
}

void APP_CLASSNAME::choose(int f,int v)
{
  if(isEmpty(f))
  {
    fields[f] = v;
  }
}

void APP_CLASSNAME::printMap()
{
  HL.clearDisplay();
  
  for(int i = 0; i<8; i++) //map
  {
    HL.setPixel(i,2,1);
    HL.setPixel(i,5,1);
    HL.setPixel(2,i,1);
    HL.setPixel(5,i,1);
  }
  for(int i=0;i<9;i++) //player
  {
    if(fields[i] == 1)
    {
      printField(i);
    }
  }
  if(ticker%20==0) //current
  {
    printField(currentField);
  }
  if(ticker%3==0) //cpu
  {
    for(int i=0;i<9;i++)
    {
      if(fields[i] == 2)
        printField(i);
    }
  }
}

void APP_CLASSNAME::printField(int n)
{
  int a; int b;
  switch(n)
  {
    case 0: a=0; b=0; break;
    case 1: a=3; b=0; break;
    case 2: a=6; b=0; break;
    case 3: a=0; b=3; break;
    case 4: a=3; b=3; break;
    case 5: a=6; b=3; break;
    case 6: a=0; b=6; break;
    case 7: a=3; b=6; break;
    case 8: a=6; b=6; break;
  }
  HL.setPixel(a,b,1); HL.setPixel(a+1,b,1);
  HL.setPixel(a,b+1,1); HL.setPixel(a+1,b+1,1);
}

int APP_CLASSNAME::randomField()
{
 //possible win?
  if((fields[0]==2)&&(fields[1]==2)&&(fields[2]==0)) return 2;
  if((fields[0]==2)&&(fields[2]==2)&&(fields[1]==0)) return 1;
  if((fields[0]==2)&&(fields[3]==2)&&(fields[6]==0)) return 6;
  if((fields[0]==2)&&(fields[4]==2)&&(fields[8]==0)) return 8;
  if((fields[0]==2)&&(fields[6]==2)&&(fields[3]==0)) return 3;
  if((fields[0]==2)&&(fields[8]==2)&&(fields[4]==0)) return 4;
  if((fields[1]==2)&&(fields[2]==2)&&(fields[0]==0)) return 0;
  if((fields[1]==2)&&(fields[4]==2)&&(fields[7]==0)) return 7;
  if((fields[1]==2)&&(fields[7]==2)&&(fields[4]==0)) return 4;
  if((fields[2]==2)&&(fields[4]==2)&&(fields[6]==0)) return 6;
  if((fields[2]==2)&&(fields[6]==2)&&(fields[4]==0)) return 4;
  if((fields[2]==2)&&(fields[8]==2)&&(fields[5]==0)) return 5;
  if((fields[3]==2)&&(fields[4]==2)&&(fields[5]==0)) return 5;
  if((fields[3]==2)&&(fields[5]==2)&&(fields[4]==0)) return 4;
  if((fields[3]==2)&&(fields[6]==2)&&(fields[0]==0)) return 0;
  if((fields[4]==2)&&(fields[5]==2)&&(fields[3]==0)) return 3;
  if((fields[4]==2)&&(fields[6]==2)&&(fields[2]==0)) return 2;
  if((fields[4]==2)&&(fields[7]==2)&&(fields[1]==0)) return 1;
  if((fields[4]==2)&&(fields[8]==2)&&(fields[0]==0)) return 0;
  if((fields[5]==2)&&(fields[8]==2)&&(fields[2]==0)) return 2;
  if((fields[6]==2)&&(fields[7]==2)&&(fields[8]==0)) return 8;
  if((fields[6]==2)&&(fields[8]==2)&&(fields[7]==0)) return 7;
  if((fields[7]==2)&&(fields[8]==2)&&(fields[6]==0)) return 6;
  if(fields[4]==0) return 4;
  //possible loss?
  if((fields[0]==1)&&(fields[1]==1)&&(fields[2]==0)) return 2;
  if((fields[0]==1)&&(fields[2]==1)&&(fields[1]==0)) return 1;
  if((fields[0]==1)&&(fields[3]==1)&&(fields[6]==0)) return 6;
  if((fields[0]==1)&&(fields[4]==1)&&(fields[8]==0)) return 8;
  if((fields[0]==1)&&(fields[6]==1)&&(fields[3]==0)) return 3;
  if((fields[0]==1)&&(fields[8]==1)&&(fields[4]==0)) return 4;
  if((fields[1]==1)&&(fields[2]==1)&&(fields[0]==0)) return 0;
  if((fields[1]==1)&&(fields[4]==1)&&(fields[7]==0)) return 7;
  if((fields[1]==1)&&(fields[7]==1)&&(fields[4]==0)) return 4;
  if((fields[2]==1)&&(fields[4]==1)&&(fields[6]==0)) return 6;
  if((fields[2]==1)&&(fields[6]==1)&&(fields[4]==0)) return 4;
  if((fields[2]==1)&&(fields[8]==1)&&(fields[5]==0)) return 5;
  if((fields[3]==1)&&(fields[4]==1)&&(fields[5]==0)) return 5;
  if((fields[3]==1)&&(fields[5]==1)&&(fields[4]==0)) return 4;
  if((fields[3]==1)&&(fields[6]==1)&&(fields[0]==0)) return 0;
  if((fields[4]==1)&&(fields[5]==1)&&(fields[3]==0)) return 3;
  if((fields[4]==1)&&(fields[6]==1)&&(fields[2]==0)) return 2;
  if((fields[4]==1)&&(fields[7]==1)&&(fields[1]==0)) return 1;
  if((fields[4]==1)&&(fields[8]==1)&&(fields[0]==0)) return 0;
  if((fields[5]==1)&&(fields[8]==1)&&(fields[2]==0)) return 2;
  if((fields[6]==1)&&(fields[7]==1)&&(fields[8]==0)) return 8;
  if((fields[6]==1)&&(fields[8]==1)&&(fields[7]==0)) return 7;
  if((fields[7]==1)&&(fields[8]==1)&&(fields[6]==0)) return 6; 
  //random

  randomSeed(millis());
  do
  {
    r = random(0,9);
  }
  while((fields[r]==2)||(fields[r]==1));
  return r;
/*
  if(fields[0]==0) return 8;
  if(fields[1]==0) return 7;
  if(fields[2]==0) return 6;
  if(fields[3]==0) return 5;
  if(fields[4]==0) return 4;
  if(fields[5]==0) return 3;
  if(fields[6]==0) return 2;
  if(fields[7]==0) return 1;
  if(fields[8]==0) return 0;
  */
}

int APP_CLASSNAME::check()
{
  if( ((fields[0]==1)&&(fields[1]==1)&&(fields[2]==1))||
      ((fields[3]==1)&&(fields[4]==1)&&(fields[5]==1))||
      ((fields[6]==1)&&(fields[7]==1)&&(fields[8]==1))||
      ((fields[0]==1)&&(fields[3]==1)&&(fields[6]==1))||
      ((fields[1]==1)&&(fields[4]==1)&&(fields[7]==1))||
      ((fields[2]==1)&&(fields[5]==1)&&(fields[8]==1))||
      ((fields[0]==1)&&(fields[4]==1)&&(fields[8]==1))||
      ((fields[2]==1)&&(fields[4]==1)&&(fields[6]==1)))
  {
    return 1;
    //Player wins
  }
  if( ((fields[0]==2)&&(fields[1]==2)&&(fields[2]==2))||
      ((fields[3]==2)&&(fields[4]==2)&&(fields[5]==2))||
      ((fields[6]==2)&&(fields[7]==2)&&(fields[8]==2))||
      ((fields[0]==2)&&(fields[3]==2)&&(fields[6]==2))||
      ((fields[1]==2)&&(fields[4]==2)&&(fields[7]==2))||
      ((fields[2]==2)&&(fields[5]==2)&&(fields[8]==2))||
      ((fields[0]==2)&&(fields[4]==2)&&(fields[8]==2))||
      ((fields[2]==2)&&(fields[4]==2)&&(fields[6]==2)))
  {
    return 2; 
    //CPU wins
  }
  if( ((fields[0]!=0)&&(fields[1]!=0)&&(fields[2]!=0)&&
       (fields[3]!=0)&&(fields[4]!=0)&&(fields[5]!=0)&&
       (fields[6]!=0)&&(fields[7]!=0)&&(fields[8]!=0)))
  {
    return 3;
    //Draw
  }
  return 0;
}

void APP_CLASSNAME::finish()
{
  
}
