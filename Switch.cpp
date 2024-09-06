/*
 * Switch.cpp
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
using namespace std;
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include <string>
#include <string.h>
#include "../inc/ST7735.h"
#include "images/images.h"
#include "LED.h"
extern uint8_t currentrow, currentcol, currentlang, enablething, level, nextGame, numMatches, lives;
string selected[] = {"nothing", "nothing"};
void printGame();
extern string correctBoard[3][2];
extern string currentBoard[6];
extern void Clock_Delay1ms(uint32_t);
void Sound_Shoot();
void Sound_Killed();
void Sound_Explosion();
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this

    IOMUX->SECCFG.PINCM[PB19INDEX] = 0x00040081; //input
    IOMUX->SECCFG.PINCM[PA18INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA17INDEX] = 0x00040081;
    IOMUX->SECCFG.PINCM[PA16INDEX] = 0x00040081;

}
// return current state of switches
uint32_t Switch_In(void){
    // write this
   int aval =  (GPIOA->DIN31_0 & ((1<<18)|(1<<16)|(1<<17)))>>16;
   int bval =  (GPIOB->DIN31_0 & (1<<19))>>19;

    if(aval == 1){
       //lang change- left
       if(currentlang==0){
           currentlang = 1;
       }
       else{
           currentlang = 0;
       }
       Clock_Delay1ms(1000);
    }
    if(aval == 2){
        if(currentrow < 2)
            currentrow++;//down
        Clock_Delay1ms(1000);
    }
    if(aval == 4){
       // LED_On((1<<24));
        //right:select

        int16_t x = 0;
        int16_t y = 0;
        if(currentcol == 0)
            x = 0;
        else
            x = 66;

        if(currentrow == 0)
            y = 0;
        else if(currentrow == 1)
            y = 57;
        else
            y = 111;

        ST7735_FillRect(x, y, 62, 52, ST7735_CYAN);
        printGame();
        if(selected[0].compare("nothing") == 0){
            selected[0] = currentBoard[currentcol*3+currentrow];


        }
        else if(selected[1].compare("nothing") == 0){
            selected[1] = currentBoard[currentcol*3+currentrow];
        //check if correct
            for(int i=0; i< 3; i++){
                for(int j=0; j< 2; j++){
                    if(selected[0].compare(correctBoard[i][j]) == 0){


                       if(j==0){
                           if(selected[1].compare(correctBoard[i][1]) == 0){
                               //match success
                               numMatches++;
                               //check if time for next board
                               if(numMatches == 3){
                                   Sound_Explosion();
                                   numMatches = 0;
                                   level++;
                                   nextGame = 1;
                                   selected[0] = "nothing";
                                   selected[1] = "nothing";
                                   Clock_Delay1ms(3000);
                                   return 0;
                               }
                               Sound_Shoot();
                               selected[0] = "nothing";
                               selected[1] = "nothing";
                               Clock_Delay1ms(1000);
                               ST7735_FillScreen(ST7735_ORANGE);
                               ST7735_DrawBitmap(63, 160, Vertical, 3, 160);
                               ST7735_DrawBitmap(0, 54, Horizontal, 128, 3);
                               ST7735_DrawBitmap(0, 108, Horizontal, 128, 3);
                               printGame();
                           }
                           else{
                               Sound_Killed();
                               //match fail
                               selected[0] = "nothing";
                               selected[1] = "nothing";
                               lives--;
                               Clock_Delay1ms(1000);
                               ST7735_FillScreen(ST7735_ORANGE);
                               ST7735_DrawBitmap(63, 160, Vertical, 3, 160);
                               ST7735_DrawBitmap(0, 54, Horizontal, 128, 3);
                               ST7735_DrawBitmap(0, 108, Horizontal, 128, 3);
                               printGame();
                               //set led & lives
                           }
                       }
                       else{

                           if(selected[1].compare(correctBoard[i][0]) == 0){
                               //match success
                              numMatches++;
                              //check if time for next board
                              if(numMatches == 3){
                                  Sound_Explosion();
                                  numMatches = 0;
                                  level++;
                                  nextGame = 1;
                                  selected[0] = "nothing";
                                  selected[1] = "nothing";
                                  Clock_Delay1ms(3000);
                                  return 0;
                              }
                                  Sound_Shoot();
                                  selected[0] = "nothing";
                                  selected[1] = "nothing";
                                  Clock_Delay1ms(1000);
                                  ST7735_FillScreen(ST7735_ORANGE);
                                  ST7735_DrawBitmap(63, 160, Vertical, 3, 160);
                                  ST7735_DrawBitmap(0, 54, Horizontal, 128, 3);
                                  ST7735_DrawBitmap(0, 108, Horizontal, 128, 3);
                                  printGame();
                           }

                          else{
                              Sound_Killed();
                              //match fail
                              selected[0] = "nothing";
                              selected[1] = "nothing";
                              lives--;
                              Clock_Delay1ms(1000);
                              ST7735_FillScreen(ST7735_ORANGE);
                              ST7735_DrawBitmap(63, 160, Vertical, 3, 160);
                              ST7735_DrawBitmap(0, 54, Horizontal, 128, 3);
                              ST7735_DrawBitmap(0, 108, Horizontal, 128, 3);
                              printGame();
                              //set led & lives
                          }
                    }

                }
            }
        }



    }
        Clock_Delay1ms(3000);
    }
    if(bval == 1 && (enablething == 1)){

        if(currentrow > 0)
            currentrow--;//up
        Clock_Delay1ms(1000);

    }

    return 0;
}
