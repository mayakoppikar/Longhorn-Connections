// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Your name
// Last Modified: 1/1/2024

#include <stdio.h>
#include <stdint.h>
#include <map>
#include <string>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);
using namespace std;

uint8_t lives = 2;
uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
float FloatPosition;  // 32-bit floating-point cm
uint32_t startTime,stopTime;
//uint32_t Flag;        // semaphore
uint32_t Offset,ADCtime,Converttime,FloatConverttime,OutFixtime,FloatOutFixtime; // in bus cycles
uint32_t Time;
uint8_t nextGame=0;
uint8_t numMatches = 0;

int enablething = 1;
uint8_t currentrow = 0;
uint8_t currentcol = 0;

uint8_t currentlang = 0;
string languages [] = {"English", "Spanish"};
SlidePot Sensor(1500,0); // copy calibration from Lab 7

string correctBoard[3][2];
string currentBoard[6];

uint8_t level = 1;




// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}


string arr1[3][2]= {
   {"dog", "cat"},
   {"broom", "sweep"},
   {"high", "low"}
};
string arr1span[3][2] = {
   {"perro", "gato"},
   {"escoba", "barrer"},
   {"alto", "bajo"}
};
string arr1rand[] = {arr1[1][0], arr1[0][0], arr1[2][1], arr1[0][1], arr1[2][0], arr1[1][1]};
string arr1randspan[] = {arr1span[1][0], arr1span[0][0], arr1span[2][1], arr1span[0][1], arr1span[2][0], arr1span[1][1]};


string arr2[3][2]= {
   {"rain", "shine"},
   {"happy", "sad"},
   {"tree", "plant"}
};

string arr2span[3][2] = {
   {"lluvia", "brillo"},
   {"feliz", "triste"},
   {"árbol", "planta"}
};

string arr2rand[] = {"happy", "plant", "rain", "tree", "sad", "shine"};
string arr2randspan[] = {"feliz", "planta", "lluvia", "árbol", "triste", "brillo"};



string arr3[3][2]= {
   {"tennis", "soccer"},
   {"kid", "child"},
   {"rose", "daisy"}
};

string arr3span[3][2] = {
   {"tenis", "fútbol"},
   {"niño", "niño"},
   {"rosa", "margarita"}
};

string arr3rand[] = {"daisy", "rose", "child", "tennis", "kid", "soccer"};
string arr3randspan[] = {"margarita", "rosa", "niño", "tenis", "niño", "fútbol"};



// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){

// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES

    if(lives == 2){
        LED_On(1<<24);
        LED_Off(1<<25);
    }
    else if(lives == 1){
        LED_On(1<<25);
        LED_Off(1<<24);
    }
    else {
        LED_Off(1<<24);
        LED_Off(1<<25);
    }


    Switch_In();
    if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
           GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
           GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
           //Time++;
           // sample 12-bit ADC1 channel 5, PB18, slidepot
           // store data into mailbox
           // set the semaphore
           Data = Sensor.In();
           Sensor.Save(Data);
           GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)

     }
}


uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(int myPhrase=0; myPhrase<= 2; myPhrase++){
    for(int myL=0; myL<= 3; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

void printGame();
// use main2 to observe graphics
int main(void){ // main2


    __disable_irq();
    PLL_Init(); // set bus speed
    LaunchPad_Init();
    ST7735_InitPrintf();
      //note: if you colors are weird, see different options for
      // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
    Sensor.Init(); //PB18 = ADC1 channel 5, slidepot
    TimerG12_IntArm(2666666,1);
    Sound_Init();  // initialize sound
      Time = 0;
      Switch_Init(); // initialize switches
      LED_Init(); // initialize LED
      __enable_irq();
    ST7735_DrawBitmap(0, 160, Home, 128, 160);

    int waiting = (GPIOB->DIN31_0 & (1<<19))>>19;

    while(waiting != 1){
        waiting = (GPIOB->DIN31_0 & (1<<19))>>19;
    }

    ST7735_FillScreen(ST7735_ORANGE);
    ST7735_DrawBitmap(20, 90, LangScreen, 96, 28);
    ST7735_DrawBitmap(20, 60, Englishs, 42, 19);

    Clock_Delay1ms(500);

    waiting = (GPIOB->DIN31_0 & (1<<19))>>19;
    enablething = 0;
    while(1){

        if(((GPIOB->DIN31_0 & (1<<19))>>19) == 1)
            break;


        Clock_Delay1ms(500);

        if(currentlang == 0){
            ST7735_DrawBitmap(20, 60, Englishs, 42, 19);
        }else{
            ST7735_DrawBitmap(20, 60, Spanishs, 43, 16);
        }

    }
    enablething = 1;

while(1){
  ST7735_FillScreen(ST7735_ORANGE);
   ST7735_DrawBitmap(63, 160, Vertical, 3, 160);
    ST7735_DrawBitmap(0, 54, Horizontal, 128, 3);
    ST7735_DrawBitmap(0, 108, Horizontal, 128, 3);

    printGame();

  while(1){
      Sensor.Sync();
        // wait for semaphore using a call to a method in Sensor
            GPIOB->DOUTTGL31_0 = RED; // toggle PB26 (minimally intrusive debugging)
            GPIOB->DOUTTGL31_0 = RED; // toggle PB26 (minimally intrusive debugging)
             // toggle red LED2 on Port B, PB26
              // convert Data to Position
            Position  = Sensor.Convert(Data);
            // move cursor to top
            ST7735_SetCursor(0,0);
            // display distance in top row OutFix
            //OutFix(Position);
            if(Position <= 1000)
                currentcol = 0;
            else
                currentcol = 1;
            printf("R,C=%d,%d",currentrow,currentcol);  // fixed point output
            ST7735_SetCursor(13,0);
            printf("Score=%d",level);
            Time++;
            GPIOB->DOUTTGL31_0 = RED; // toggle PB26 (minimally intrusive debugging)
            if(nextGame){
                nextGame = 0;
                break;
            }

  }
}
}

// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
    // write code to test switches and LEDs

      int x = GPIOA->DIN31_0 & ((1<<16) | (1<<17) | (1<<18));
      x = x | GPIOB->DIN31_0 & (1<<19);
      if(x){
          LED_On((1<<24) | (1<<25) | (1<<26));
      }
      else{
          LED_Off((1<<24) | (1<<25) | (1<<26));
      }

  }
}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  //Sound_Killed();

  while(1){
      /*
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
    */
  }
}
// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main5(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  __enable_irq();

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch


  }
}

void printGame(){
    if((currentlang == 0) && (level == 1)){
            ST7735_SetCursor(4,2);
            printf("%s", arr1rand[0].c_str());
            ST7735_SetCursor(4,7);
            printf("%s", arr1rand[1].c_str());
            ST7735_SetCursor(4,13);
            printf("%s", arr1rand[2].c_str());
            ST7735_SetCursor(13,2);
            printf("%s", arr1rand[3].c_str());
            ST7735_SetCursor(13,7);
            printf("%s", arr1rand[4].c_str());
            ST7735_SetCursor(13,13);
            printf("%s", arr1rand[5].c_str());
            for(int x = 0; x < 3; x++){
                for(int y = 0; y < 2; y++){
                    correctBoard[x][y] = arr1[x][y];
                }
            }
            for(int x = 0; x < 6; x++){
                currentBoard[x] = arr1rand[x];
            }
        }
        else if((currentlang == 1) && (level == 1)){
            ST7735_SetCursor(4,2);
            printf("%s", arr1randspan[0].c_str());
            ST7735_SetCursor(4,7);
            printf("%s", arr1randspan[1].c_str());
            ST7735_SetCursor(4,13);
            printf("%s", arr1randspan[2].c_str());
            ST7735_SetCursor(13,2);
            printf("%s", arr1randspan[3].c_str());
            ST7735_SetCursor(13,7);
            printf("%s", arr1randspan[4].c_str());
            ST7735_SetCursor(13,13);
            printf("%s", arr1randspan[5].c_str());
            for(int x = 0; x < 3; x++){
                for(int y = 0; y < 2; y++){
                    correctBoard[x][y] = arr1span[x][y];
                }
            }
            for(int x = 0; x < 6; x++){
                currentBoard[x] = arr1randspan[x];
            }

        }
        else if((currentlang == 0) && (level == 2)){
            ST7735_SetCursor(4,2);
            printf("%s", arr2rand[0].c_str());
            ST7735_SetCursor(4,7);
            printf("%s", arr2rand[1].c_str());
            ST7735_SetCursor(4,13);
            printf("%s", arr2rand[2].c_str());
            ST7735_SetCursor(13,2);
            printf("%s", arr2rand[3].c_str());
            ST7735_SetCursor(13,7);
            printf("%s", arr2rand[4].c_str());
            ST7735_SetCursor(13,13);
            printf("%s", arr2rand[5].c_str());
            for(int x = 0; x < 3; x++){
                for(int y = 0; y < 2; y++){
                    correctBoard[x][y] = arr2[x][y];
                }
            }
            for(int x = 0; x < 6; x++){
                currentBoard[x] = arr2rand[x];
            }
        }
        else if((currentlang == 1) && (level == 2)){
            ST7735_SetCursor(4,2);
            printf("%s", arr2randspan[0].c_str());
            ST7735_SetCursor(4,7);
            printf("%s", arr2randspan[1].c_str());
            ST7735_SetCursor(4,13);
            printf("%s", arr2randspan[2].c_str());
            ST7735_SetCursor(13,2);
            printf("%s", arr2randspan[3].c_str());
            ST7735_SetCursor(13,7);
            printf("%s", arr2randspan[4].c_str());
            ST7735_SetCursor(13,13);
            printf("%s", arr2randspan[5].c_str());
            for(int x = 0; x < 3; x++){
                for(int y = 0; y < 2; y++){
                    correctBoard[x][y] = arr2span[x][y];
                }
            }
            for(int x = 0; x < 6; x++){
                currentBoard[x] = arr2randspan[x];
            }
        }
        else if((currentlang == 0) && (level == 3)){
            ST7735_SetCursor(4,2);
            printf("%s", arr3rand[0].c_str());
            ST7735_SetCursor(4,7);
            printf("%s", arr3rand[1].c_str());
            ST7735_SetCursor(4,13);
            printf("%s", arr3rand[2].c_str());
            ST7735_SetCursor(13,2);
            printf("%s", arr3rand[3].c_str());
            ST7735_SetCursor(13,7);
            printf("%s", arr3rand[4].c_str());
            ST7735_SetCursor(13,13);
            printf("%s", arr3rand[5].c_str());
            for(int x = 0; x < 3; x++){
                for(int y = 0; y < 2; y++){
                    correctBoard[x][y] = arr3[x][y];
                }
            }
            for(int x = 0; x < 6; x++){
                currentBoard[x] = arr3rand[x];
            }
        }
        else if((currentlang == 1) && (level == 3)){
            ST7735_SetCursor(4,2);
            printf("%s", arr3randspan[0].c_str());
            ST7735_SetCursor(4,7);
            printf("%s", arr3randspan[1].c_str());
            ST7735_SetCursor(4,13);
            printf("%s", arr3randspan[2].c_str());
            ST7735_SetCursor(13,2);
            printf("%s", arr3randspan[3].c_str());
            ST7735_SetCursor(13,7);
            printf("%s", arr3randspan[4].c_str());
            ST7735_SetCursor(13,13);
            printf("%s", arr3randspan[5].c_str());
            for(int x = 0; x < 3; x++){
                for(int y = 0; y < 2; y++){
                    correctBoard[x][y] = arr3span[x][y];
                }
            }
            for(int x = 0; x < 6; x++){
                currentBoard[x] = arr3randspan[x];
            }
        }
}
