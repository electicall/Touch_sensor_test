#include <stdint.h>
#include <LCD.h>
#include <SPI.h>
#include <XPT2046.h>
#include "Touch.h"
int globx, globy;
int bx[8]={0,120,0,120,0,120,0,120};
int by[8]={161,161,201,201,241,241,281,281};
char butontext[]="12345678901234";
int temp_pan;
void setup()
{ 
   __SD_CS_DISABLE();
    Serial.begin(9600);
    SPI.setDataMode(SPI_MODE3);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.begin();

    Tft.lcd_init();                                      // init TFT library
    Tp.tp_init();
    Tp.tp_adjust_def();
    Tft.lcd_clear_screen(BLACK);
    Tft.lcd_display_string(20, 20, (const uint8_t *)"Test On", FONT_1608, YELLOW);
    Tft.lcd_draw_rect(0,0,239,159,YELLOW); // galvenais info logs
  
    Tp.tp_draw_button(1,2,"UP", GREEN);
    Tp.tp_draw_button(2,4,"DOWN", GREEN);
    Tp.tp_draw_button(3,5,"LEFT", GREEN);
    Tp.tp_draw_button(4,5,"RIGHT", GREEN);
    Tp.tp_draw_button(5,2,"OK", WHITE);
    Tp.tp_draw_button(6,6,"CANCEL", RED);
    Tp.tp_draw_button(7,4,"MENU", YELLOW);
    Tp.tp_draw_button(8,4,"EXIT", YELLOW);
    
   }


void loop()
{
    //Tp.tp_draw_board();
    //globx=Tp.tp_get_x();
   // globy=Tp.tp_get_y();
    //Serial.write(12);
    //Serial.print((int)globx);
    //Serial.println();
//    while(temp_pan ==0){
//    temp_pan=Tp.tp_is_button();
//    }
//    if (temp_pan>0){    
//    Serial.print("Button nr: ");
//    Serial.print(temp_pan);
//    Serial.print(" pressed");
//    Serial.println();}
//    while(temp_pan !=0) {
//      temp_pan=Tp.tp_is_button();
//    }
//    Serial.println("Button released");
    temp_pan=Tp.tp_is_button();
    Serial.print("Button: ");
    Serial.print(temp_pan);
    Serial.println();
   
}

