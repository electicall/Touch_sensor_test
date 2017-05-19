#include <Wire.h>
#include "Adafruit_Trellis.h"

Adafruit_Trellis matrix0 = Adafruit_Trellis();
Adafruit_Trellis matrix1 = Adafruit_Trellis();
Adafruit_TrellisSet trellis =  Adafruit_TrellisSet(&matrix0, &matrix1);
#define NUMTRELLIS 2
#define numKeys (NUMTRELLIS * 16)
#define INTPIN A8
byte button_matrix[8]={0,1,2,3,16,17,18,19};
int sequence[32] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
trellis.begin(0x71, 0x70);

//nodzeesh iespeejamaas diozhu gaismas
 for (uint8_t i=0; i<numKeys; i++) 
   {
     trellis.clrLED(i);
     trellis.writeDisplay();
     delay(40);
   }

}

void loop() {
  // put your main code here, to run repeatedly:
ReadTrellisButtons();

}

void ReadTrellisButtons()
  {
    delay(20);
    if (trellis.readSwitches()) 
      {
        // go through every button
        for (uint8_t i=0; i<numKeys; i++) 
          {
          // if it was pressed...
            if (trellis.justPressed(i)) 
              {
                Serial.print("v"); Serial.println(i);
                // Alternate the LED
                if (trellis.isLED(i))
                  {
                    trellis.clrLED(i);
                  }
                else
                  {
                    trellis.setLED(i);
                  }
                //mainam sekvencees mainīgo
                if (sequence[i]==1)
                  {
                    sequence[i]=0;
                  }
                else
                  {
                    sequence[i]=1;
                  }
              } 
          }
      // tell the trellis to set the LEDs we requested
      trellis.writeDisplay();
      for (int j=0;j<4;j++)
      {
        for (int k=0;k<32;k=k+4)
        {
        Serial.print(sequence[j+k]);
        }
        Serial.println();
      }
      }
  } 

