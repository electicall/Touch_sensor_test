//pamatbibliotēkas
#include <stdint.h>
#include <Fluxamasynth.h>
#include <NewSoftSerial.h>
#include <PgmChange.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Trellis.h>
//Ekrāna bibliotēkas, lokāli, paredzēts bibliotēku editēšana
#include "HX8347D.h"
#include "XPT2046.h"
#include "Touch.h"
//datu biblioteeka, kur glabaajas char masiivi ar tekstu
#include "inames.h"

//Fluxama synth, tiks atmests, kad būs tīri midi darbinaashana
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

int transpose_mod=0;
int last_transpose_mod=0;

byte button_matrix[8]={4,3,2,1,20,19,18,17};
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
      //notes pointers
      PGM_P const note_table [] PROGMEM ={n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11};
const int PIN_SD_CS = 5;    

//notis C4
byte notes[32]={41,40,38,36,48,47,45,43,55,53,52,50,62,60,59,57,65,64,62,60,72,71,69,67,79,77,76,74,86,84,83,81};
byte notes2[32]={57,59,60,62,81,83,84,86,50,52,53,55,74,76,77,79,43,45,47,48,67,69,71,72,36,38,40,41,60,62,64,65};

byte drums[16]={46,42,38,35,39,44,40,36,50,48,47,41,55,53,51,49};
byte lastprog=0;
byte lastlastprog=0;
int temp_pan=0;
//pogu staavoklji
byte last_state=0;
byte current_state;
void setup()
{ 
Serial.begin(9600);
__XPT2046_CS_DISABLE();

 pinMode(PIN_SD_CS,OUTPUT);
    digitalWrite(PIN_SD_CS,HIGH);
    Sd2Card card;
    card.init(SPI_FULL_SPEED, PIN_SD_CS); 
    if(!SD.begin(PIN_SD_CS))  { 
        Serial.println("failed!");
        while(1);                               // init fail, die here
    }


SPI.setDataMode(SPI_MODE3);
SPI.setBitOrder(MSBFIRST);
SPI.setClockDivider(SPI_CLOCK_DIV4);
SPI.begin();


Serial.begin(9600);
//Noreseto sintiņu
  synth.midiReset();  
  synth.pitchBendRange(0, 4);
  synth.programChange(0, 1, lastprog);
//saak taustinjus
  trellis.begin(0x70, 0x71);


Tft.lcd_init(); // incializē lcd 
Tp.tp_init(); //inicializē touchpadu
Tp.tp_adjust_def(); //autokalibracija
//galvenais izkartojums
Tft.lcd_clear_screen(BLACK); 
Tft.lcd_draw_rect(0,0,239,159,YELLOW); // galvenais info logs
Tft.lcd_display_string(2, 1, (const uint8_t *) "      BANK:", FONT_1608, YELLOW);
Tft.lcd_display_string(2, 16, (const uint8_t *)"INSTRUMENT:", FONT_1608, YELLOW);
Tft.lcd_display_string(2, 31, (const uint8_t *)" PLAY MODE:", FONT_1608,YELLOW);
draw_layout_grid();
draw_layout(); //ziiimee nosu izkartojumu
//draw_instruments(0);//ziimee instrumentus, attieciigi atbilstoshi bankai.
//pamatpogu ziimeeshana
Tp.tp_draw_button(1,7,"BANK UP", GREEN);
Tp.tp_draw_button(2,9,"INSTR. UP", GBLUE);
Tp.tp_draw_button(3,9,"BANK DOWN", GREEN);
Tp.tp_draw_button(4,11,"INSTR. DOWN", GBLUE);
Tp.tp_draw_button(5,11,"TRANSPOSE +", WHITE);
Tp.tp_draw_button(8,4,"MENU", YELLOW);
Tp.tp_draw_button(7,11,"TRANSPOSE -", WHITE);
Tp.tp_draw_button(6,12,"DRUM MACHINE", GREEN);
DrawIn();
Draw_drum_status();

//inicializācijas pogu vizualizacijas cikli
// for (int i=0;i<20;i++)
//   {
//     for (int j=0;j<4;j++)
//       {
//         if (i<=16)
//           {
//             trellis.setLED((button_matrix[i/2]+j*4)-1);
//             trellis.writeDisplay();
//           }
//         else
//           {
//             if (j%2==0) 
//               {
//                 digitalWrite(3,HIGH);
//               }
//             else 
//               {
//                 digitalWrite(3,LOW);
//               }
//           }
//       delay(20);
//       }
//     }

//nodzeesh iespeejamaas diozhu gaismas
 for (uint8_t i=0; i<numKeys/2; i++) 
   {
     trellis.clrLED(i);
     trellis.clrLED(i+16);
     trellis.writeDisplay();
     delay(40);
   }
    
   }


void loop()
{
delay(20); //nepieciešama minimaalaa pauze, lai korekti lasiitu no trellis pogaam
      
if (trellis.readSwitches()) 
      {
        check_buttons();
      }
// pajautaa vai nav nospiesta kaada no definētām pogaam touchpadam (atgriezh pogas kaartas numura veertiibu)    
    temp_pan=Tp.tp_is_button();
    if (temp_pan !=0) 
      {
        current_state=temp_pan;
        if (current_state != last_state)
          {
            last_state=current_state;  
            Menu_button_processing(last_state); 
          }
      }
    else last_state=0;

}

void clear_layout(){
  Tft.lcd_fill_rect(75,70,159,80,BLACK);
}
void draw_layout() 
{

  byte zzx=0;
  int zz;
  int trans=0;
  
  for (int yy=70;yy<150;yy=yy+20)
      {
      for (int xx=75;xx<235;xx=xx+20)
        {
          

          if (xx>135) {zz=xx+2;}
          else zz=xx;
          //Tft.lcd_draw_rect(zz,yy,20,20,WHITE);
          //paarziimee ieprieksheejo transponeto ar melnu
          trans=((notes2[zzx]+last_transpose_mod )%12);
          char * ptrx = (char *) pgm_read_word (&note_table [trans]);
          char bufferx [2]; // must be large enough!
          strcpy_P (bufferx, ptrx);
          Tft.lcd_display_string(zz+2,yy+2,(const uint8_t *)bufferx, FONT_1608,BLACK);
          //ziimee tekosho transponeeto
          trans=((notes2[zzx]+transpose_mod )%12);
          ptrx = (char *) pgm_read_word (&note_table [trans]);
          bufferx [2]; // must be large enough!
          strcpy_P (bufferx, ptrx);
          Tft.lcd_display_string(zz+2,yy+2,(const uint8_t *)bufferx, FONT_1608,YELLOW);
          /* Serial.print("draw_layout");
          Serial.print(zzx);
          Serial.print(" : ");
          Serial.println(lastprog); */
          zzx++;
       
        }
    }
    int lenght= numdigits(last_transpose_mod);
    Tft.lcd_display_num(106,46,abs(last_transpose_mod),lenght,FONT_1608,BLACK);
    lenght= numdigits(transpose_mod);
    Tft.lcd_display_num(106,46,abs(transpose_mod),lenght,FONT_1608,WHITE);
    if (transpose_mod<0) 
      {
        Tft.lcd_display_string(96,46,"-", FONT_1608,WHITE);
      }
    if (transpose_mod>0) 
      {
        Tft.lcd_display_string(96,46,"+", FONT_1608,WHITE);
      }
    if (transpose_mod==0 && last_transpose_mod<0)
      {
        Tft.lcd_display_string(96,46,"-", FONT_1608,BLACK);
      }
    if (transpose_mod==0 && last_transpose_mod>0)
      {
        Tft.lcd_display_string(96,46,"+", FONT_1608,BLACK);
      }  
     
}

void draw_layout_grid() 
{
  Tft.lcd_display_string(2,46,(const uint8_t *)" TRANSPOSE:", FONT_1608,YELLOW);
  byte zzx=0;
  int zz;
 
  for (int yy=70;yy<150;yy=yy+20)
      {
      Tft.lcd_display_string(10,yy+5,"Instrument", FONT_1206,YELLOW);
      Tft.lcd_display_string(2,yy+5,"+", FONT_1206,YELLOW);
      for (int xx=75;xx<235;xx=xx+20)
        {
          

          if (xx>135) {zz=xx+2;}
          else zz=xx;
          Tft.lcd_draw_rect(zz,yy,20,20,WHITE);
          zzx++;
       
        }
    }
}



void Menu_button_processing(byte value){
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
      else if (value==5 and transpose_mod < 11){
          last_transpose_mod=transpose_mod;
          transpose_mod++;
         //clear_layout();
          draw_layout();
          
      }
      else if (value==7 and transpose_mod > -11){
          last_transpose_mod=transpose_mod;
          transpose_mod--;
          //clear_layout();
          draw_layout();
      }
      else if (value==6){
          drumm=!drumm;
          Draw_drum_status();
          }
}
void check_buttons(){
  for (uint8_t i=0; i<numKeys; i++) 
        {
          if (trellis.justPressed(i)) 
            {
              trellis.setLED(i);
              if (drumm!=true) 
                {
                  synth.noteOn(1, notes[i]-12+transpose_mod, 127);
                }
              else 
                {
                  synth.noteOn(9,drums[i],127);
                }
            }
          if (trellis.justReleased(i)) 
            {
              trellis.clrLED(i);
              synth.noteOff(1, notes[i]-12+transpose_mod);
            }
          }
  trellis.writeDisplay();
}

void DrawIn(){

//ieraksta instrumentu  dzēš
      char * ptr = (char *) pgm_read_word (&instr_table [lastlastprog]);
      char buffer [18]; 
      strcpy_P (buffer, ptr); 
      Tft.lcd_display_string(96, 16, (const uint8_t *)buffer, FONT_1608, BLACK);
//ieraksta banku dzēš
      ptr = (char *) pgm_read_word (&bank_table [lastlastprog/8]);
      strcpy_P (buffer, ptr);
      Tft.lcd_display_string(96, 1, (const uint8_t *)buffer, FONT_1608, BLACK);


//ieraksta instrumentu  
      ptr = (char *) pgm_read_word (&instr_table [lastprog]);
      strcpy_P (buffer, ptr); 
      Tft.lcd_display_string(96, 16, (const uint8_t *)buffer, FONT_1608, WHITE);
//ieraksta banku      
      ptr = (char *) pgm_read_word (&bank_table [lastprog/8]);
      strcpy_P (buffer, ptr);
      Tft.lcd_display_string(96, 1, (const uint8_t *)buffer, FONT_1608, WHITE);
      lastlastprog=lastprog;
}

void Draw_drum_status(){
  if (drumm) 
    {
      Tft.lcd_display_string(96,31,(const uint8_t *)"Melody",FONT_1608,BLACK);
      Tft.lcd_display_string(96,31,(const uint8_t *)"Drums",FONT_1608,CYAN);
    }
  else 
    {
      Tft.lcd_display_string(96,31,(const uint8_t *)"Drums",FONT_1608,BLACK);
      Tft.lcd_display_string(96,31,(const uint8_t *)"Melody",FONT_1608,CYAN);
    } 

    
}

int numdigits(int i)
{
       char str[20];

       sprintf(str,"%d",abs(i));
       return(strlen(str));
} 

