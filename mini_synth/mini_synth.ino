#include <stdint.h>
#include <Fluxamasynth.h>
#include <NewSoftSerial.h>
#include <PgmChange.h>
#include "Adafruit_Trellis.h"
#include "inames.h"
#include <LCD.h>
#include <SPI.h>
#include <XPT2046.h>
#include "Touch.h"
#include <String.h>

Fluxamasynth synth = Fluxamasynth();
//pogu matricas
#define MOMENTARY 0
#define LATCHING 1
#define MODE MOMENTARY
Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_Trellis matrix1 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0, &matrix1);
#define NUMTRELLIS 2
#define numKeys (NUMTRELLIS * 16)
#define INTPIN A2
int button_matrix[8]={4,3,2,1,20,19,18,17};
//katra nākamā rinda ir +4
bool drumm=false;

 //inicializējam norades uz stringiem
      //drums pointers
      PGM_P const drum_table[] PROGMEM= { d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15,d16,d17,d18,d19,d20,
      d21,d22,d23,d24,d25,d26,d27,d28,d29,d30,d31,d32,d33,d34,d35,d36,d37,d38,d39,d40,d41,d42,d43,d44,d45,d46};
      //bank pointers
      PGM_P const bank_table[] PROGMEM= {i0,i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15};
      //instrument pointers
      PGM_P const instr_table[] PROGMEM= {s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20,
      s21,s22,s23,s24,s25,s26,s27,s28,s29,s30,s31,s32,s33,s34,s35,s36,s37,s38,s39,s40,s41,s42,s43,s44,s45,s46,s47,s48,
      s49,s50,s51,s52,s53,s54,s55,s56,s57,s58,s59,s60,s61,s62,s63,s64,s65,s66,s67,s68,s69,s70,s71,s72,s73,s74,s75,s76,
      s77,s78,s79,s80,s81,s82,s83,s84,s85,s86,s87,s88,s89,s90,s91,s92,s93,s94,s95,s96,s97,s98,s99,s100,s101,s102,s103,
      s104,s105,s106,s107,s108,s109,s110,s111,s112,s113,s114,s115,s116,s117,s118,s119,s120,s121,s122,s123,s124,s125,s126,s127};

//notis nosaukumi
String notis[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

//notis C4
int notes[32]={41,40,38,36,48,47,45,43,55,53,52,50,62,60,59,57,65,64,62,60,72,71,69,67,79,77,76,74,86,84,83,81};
int drums[16]={46,42,38,35,39,44,40,36,50,48,47,41,55,53,51,49};
bool drum=false;
int lastprog=0;

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
    Serial.begin(9600);
      synth.midiReset();  
      synth.pitchBendRange(0, 4);
      synth.programChange(0, 1, lastprog);
      trellis.begin(0x70, 0x71);

        for (int i=0;i<20;i++)
  {
    for (int j=0;j<4;j++)
    {
      if (i<=16){
      trellis.setLED((button_matrix[i/2]+j*4)-1);
      trellis.writeDisplay();}
      else{
      if (j%2==0) {digitalWrite(3,HIGH);}
      else {digitalWrite(3,LOW);}
      }
      delay(20);
    }
  }

 
  for (uint8_t i=0; i<numKeys/2; i++) {
    trellis.clrLED(i);
    trellis.clrLED(i+16);
    trellis.writeDisplay();
    delay(40);
  }
    
    
    Tft.lcd_init();                                      // init TFT library
    Tp.tp_init();
    Tp.tp_adjust_def();
    Tft.lcd_clear_screen(BLACK);
    Tft.lcd_draw_rect(0,0,239,159,YELLOW); // galvenais info logs
    Tft.lcd_display_string(2, 2, (const uint8_t *)"v1.1  BANK:", FONT_1608, YELLOW);
    Tft.lcd_display_string(2, 22, (const uint8_t *)"INSTRUMENT:", FONT_1608, YELLOW);
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
   
    DrawIn();
    temp_pan=0;
    
   }


void loop()
{
     delay(20);
      
      if (trellis.readSwitches()) 
      {
        check_buttons();
      }
    
    temp_pan=Tp.tp_is_button();
    if (temp_pan !=0) 
      {
          current_state=temp_pan;
          if (current_state != last_state)
          {
          last_state=current_state;  
         changeprog(last_state); 
         
       }
      }
     else last_state=0;

   
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
   
    Tft.lcd_display_string(96, 2, (const uint8_t *)"Pianos", FONT_1608, YELLOW);
    Tft.lcd_display_string(96, 22, (const uint8_t *)"1234567890123456", FONT_1608, YELLOW);  
}

void changeprog(int value){
// value 0 - instruemnt -- , value 1 - instrument ++ , value 2 - bank -- , value 3 - bank ++
      if (value==2 and lastprog<127){
          lastprog++;
          DrawIn();
          synth.programChange(0, 1, lastprog);
      }
      else if (value==4 and lastprog>0){
          lastprog--;
          DrawIn();
          synth.programChange(0, 1, lastprog);
      }
      else if (value==3 and lastprog>7){
          lastprog=((lastprog/8)-1)*8;
          DrawIn();
          synth.programChange(0, 1, lastprog);
      }
      else if (value==1 and lastprog<120){
          lastprog=((lastprog/8)+1)*8;
          DrawIn();
          synth.programChange(0, 1, lastprog);
      }
}
void check_buttons(){
  for (uint8_t i=0; i<numKeys; i++) 
        {
          if (trellis.justPressed(i)) 
            {
              trellis.setLED(i);
              if (drumm!=true) {
              synth.noteOn(1, notes[i]-12, 127);
              }
              else {synth.noteOn(9,drums[i],127);}
            }
          if (trellis.justReleased(i)) 
            {
              trellis.clrLED(i);
              synth.noteOff(1, notes[i]-12);
            }
          }
          trellis.writeDisplay();
}

void DrawIn(){
      Tft.lcd_fill_rect(96,1,143,40,BLACK);
//ieraksta instrumentu  
      char * ptr = (char *) pgm_read_word (&instr_table [lastprog]);
      char buffer [18]; // must be large enough!
      strcpy_P (buffer, ptr);
      Tft.lcd_display_string(96, 22, (const uint8_t *)buffer, FONT_1608, YELLOW);
//ieraksta banku      
      ptr = (char *) pgm_read_word (&bank_table [lastprog/8]);
      strcpy_P (buffer, ptr);
      Tft.lcd_display_string(96, 2, (const uint8_t *)buffer, FONT_1608, YELLOW);
}

