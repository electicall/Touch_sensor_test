#include <stdint.h>
#include <LCD.h>
#include <SPI.h>
#include <XPT2046.h>
#include "Touch.h"
int temp_pan;
int last_state=0;
int current_state;
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
    Tft.lcd_draw_rect(0,0,239,159,YELLOW); // galvenais info logs
    draw_layout(0);
    draw_instruments(0);
    Tp.tp_draw_button(1,7,"BANK UP", GREEN);
    Tp.tp_draw_button(2,9,"INSTR. UP", GBLUE);
    Tp.tp_draw_button(3,9,"BANK DOWN", GREEN);
    Tp.tp_draw_button(4,11,"INSTR. DOWN", GBLUE);
    Tp.tp_draw_button(5,9,"VOLUME UP", WHITE);
    Tp.tp_draw_button(6,4,"MENU", YELLOW);
    Tp.tp_draw_button(7,11,"VOLUME DOWN", WHITE);
    //Tp.tp_draw_button(8,1," ", YELLOW);
   
    
    temp_pan=0;
    
   }


void loop()
{
    temp_pan=Tp.tp_is_button();
    if (temp_pan !=0) 
      {
          current_state=temp_pan;
          if (current_state != last_state)
          {
          last_state=current_state;  
         //poga nospiesta, trigeris. 
          }
      }

   
}

void draw_layout(int transpose) 
{
  int z;
  for (int y=70;y<150;y=y+20)
      {
      Tft.lcd_display_string(10,y+5,"Instrument", FONT_1206,YELLOW);
      Tft.lcd_display_string(2,y+5,"+", FONT_1206,YELLOW);
      for (int x=75;x<235;x=x+20)
        {
          if (x>135) {z=x+2;}
          else z=x;
          Tft.lcd_draw_rect(z,y,20,20,WHITE);
          Tft.lcd_display_string(z+2,y+2,"C#", FONT_1608,YELLOW);
       
        }
    }
}

void draw_instruments(int instrument)
{
    Tft.lcd_display_string(2, 2, (const uint8_t *)"v1.1  BANK:", FONT_1608, YELLOW);
    Tft.lcd_display_string(2, 22, (const uint8_t *)"INSTRUMENT:", FONT_1608, YELLOW);
    Tft.lcd_display_string(96, 2, (const uint8_t *)"Pianos", FONT_1608, YELLOW);
    Tft.lcd_display_string(96, 22, (const uint8_t *)"1234567890123456", FONT_1608, YELLOW);  
}

