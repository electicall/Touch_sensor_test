
#include "Touch.h"
#include <LCD.h>
#include <XPT2046.h>
#include <stdlib.h>
#include <math.h>
//#include <Serial.h>

int color=YELLOW;

void TP::tp_init(void)
{
	Xpt.xpt2046_init();
	//Serial.begin(9600, SERIAL_8N1);
}

void TP::tp_draw_touch_point(uint16_t hwXpos, uint16_t hwYpos, uint16_t hwColor)
{
	Tft.lcd_draw_line(hwXpos - 12, hwYpos, hwXpos + 13, hwYpos, hwColor);
	Tft.lcd_draw_line(hwXpos, hwYpos - 12, hwXpos, hwYpos + 13, hwColor);
	Tft.lcd_draw_point(hwXpos + 1, hwYpos + 1, hwColor);
	Tft.lcd_draw_point(hwXpos - 1, hwYpos + 1, hwColor);
	Tft.lcd_draw_point(hwXpos + 1, hwYpos - 1, hwColor);
	Tft.lcd_draw_point(hwXpos - 1, hwYpos - 1, hwColor);
	Tft.lcd_draw_circle(hwXpos, hwYpos, 6, hwColor);
}

void TP::tp_draw_big_point(uint16_t hwXpos, uint16_t hwYpos, uint16_t hwColor)
{
	Tft.lcd_draw_point(hwXpos, hwYpos, hwColor);
	Tft.lcd_draw_point(hwXpos + 1, hwYpos, hwColor);
	Tft.lcd_draw_point(hwXpos, hwYpos + 1, hwColor);
	Tft.lcd_draw_point(hwXpos + 1, hwYpos + 1, hwColor);
}

void TP::tp_show_info(uint16_t hwXpos0, uint16_t hwYpos0,
                     uint16_t hwXpos1, uint16_t hwYpos1,
                     uint16_t hwXpos2, uint16_t hwYpos2,
                     uint16_t hwXpos3, uint16_t hwYpos3, uint16_t hwFac)
{

	Tft.lcd_display_string(40, 160, (const uint8_t *)"x1", 16, RED);
	Tft.lcd_display_string(40 + 80, 160, (const uint8_t *)"y1", 16, RED);

	Tft.lcd_display_string(40, 180, (const uint8_t *)"x2", 16, RED);
	Tft.lcd_display_string(40 + 80, 180, (const uint8_t *)"y2", 16, RED);

	Tft.lcd_display_string(40, 200, (const uint8_t *)"x3", 16, RED);
	Tft.lcd_display_string(40 + 80, 200, (const uint8_t *)"y3", 16, RED);

	Tft.lcd_display_string(40, 220, (const uint8_t *)"x4", 16, RED);
	Tft.lcd_display_string(40 + 80, 220, (const uint8_t *)"y4", 16, RED);

	Tft.lcd_display_string(40, 240, (const uint8_t *)"fac is:", 16, RED);

	Tft.lcd_display_num(40 + 24, 160, hwXpos0, 4, 16, RED);
	Tft.lcd_display_num(40 + 24 + 80, 160, hwYpos0, 4, 16, RED);

	Tft.lcd_display_num(40 + 24, 180, hwXpos1, 4, 16, RED);
	Tft.lcd_display_num(40 + 24 + 80, 180, hwYpos1, 4, 16, RED);

	Tft.lcd_display_num(40 + 24, 200, hwXpos2, 4, 16, RED);
	Tft.lcd_display_num(40 + 24 + 80, 200, hwYpos2, 4, 16, RED);

	Tft.lcd_display_num(40 + 24, 220, hwXpos3, 4, 16, RED);
	Tft.lcd_display_num(40 + 24 + 80, 220, hwYpos3, 4, 16, RED);

	Tft.lcd_display_num(40 + 56, 240, hwFac, 3, 16, RED);
}

uint8_t TP::tp_scan(uint8_t chCoordType)
{
	if (!(__XPT2046_IRQ_READ())) {
		if (chCoordType) {
			Xpt.xpt2046_twice_read_xy(&s_tTouch.hwXpos, &s_tTouch.hwYpos);
		} else if (Xpt.xpt2046_twice_read_xy(&s_tTouch.hwXpos, &s_tTouch.hwYpos)) {
			//s_tTouch.hwXpos = 0.066 * s_tTouch.hwXpos + (-12);//s_tTouch.fXfac * s_tTouch.hwXpos + s_tTouch.iXoff;
			//s_tTouch.hwYpos = (-0.089) * s_tTouch.hwYpos + (352);//s_tTouch.fYfac * s_tTouch.hwYpos + s_tTouch.iYoff;
			s_tTouch.hwXpos = s_tTouch.fXfac * s_tTouch.hwXpos + s_tTouch.iXoff;
			s_tTouch.hwYpos = s_tTouch.fYfac * s_tTouch.hwYpos + s_tTouch.iYoff;
		}
		if (0 == (s_tTouch.chStatus & TP_PRESS_DOWN)) {
			s_tTouch.chStatus = TP_PRESS_DOWN | TP_PRESSED;
			s_tTouch.hwXpos0 = s_tTouch.hwXpos;
			s_tTouch.hwYpos0 = s_tTouch.hwYpos;
		} 

	} else {
		if (s_tTouch.chStatus & TP_PRESS_DOWN) {
			s_tTouch.chStatus &= ~(1 << 7);
		} else {
			s_tTouch.hwXpos0 = 0;
			s_tTouch.hwYpos0 = 0;
			s_tTouch.hwXpos = 0xffff;
			s_tTouch.hwYpos = 0xffff;
		}
	}

	return (s_tTouch.chStatus & TP_PRESS_DOWN);
}


void TP::tp_adjust(void)
{	
	uint8_t  cnt = 0;
	uint16_t hwTimeout = 0, d1, d2, pos_temp[4][2];
	uint32_t tem1, tem2;
	float fac;				

	Tft.lcd_clear_screen(WHITE);
	Tft.lcd_display_string(40, 40, (const uint8_t *)"Please use the stylus click the cross on the screen. The cross will always move until the screen adjustment is completed.",
					16, RED);
	tp_draw_touch_point(20, 20, RED);
	s_tTouch.chStatus = 0;
	s_tTouch.fXfac = 0;
	while (1) {
		tp_scan(1);
		if((s_tTouch.chStatus & 0xC0) == TP_PRESSED) {	
			hwTimeout = 0;
			s_tTouch.chStatus &= ~(1 << 6);
						   			   
			pos_temp[cnt][0] = s_tTouch.hwXpos;
			pos_temp[cnt][1] = s_tTouch.hwYpos;
			cnt ++;	  
			switch(cnt) {			   
				case 1:						 
					tp_draw_touch_point(20, 20, WHITE);
					tp_draw_touch_point(LCD_WIDTH - 20, 20, RED);
					break;
				case 2:
					tp_draw_touch_point(LCD_WIDTH - 20, 20, WHITE);
					tp_draw_touch_point(20, LCD_HEIGHT - 20, RED);
					break;
				case 3:
					tp_draw_touch_point(20, LCD_HEIGHT - 20, WHITE);
					tp_draw_touch_point(LCD_WIDTH - 20, LCD_HEIGHT - 20, RED);
					break;
				case 4:	
					tem1=abs((int16_t)(pos_temp[0][0]-pos_temp[1][0]));//x1-x2
					tem2=abs((int16_t)(pos_temp[0][1]-pos_temp[1][1]));//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					tem1+=tem2;
					d1=sqrt(tem1);

					tem1=abs((int16_t)(pos_temp[2][0]-pos_temp[3][0]));//x3-x4
					tem2=abs((int16_t)(pos_temp[2][1]-pos_temp[3][1]));//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					tem1+=tem2;
					d2=sqrt(tem1);
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0) {
						cnt=0;
						Serial.print(d1, DEC);
						Serial.print("\t"); 
						Serial.print(d2, DEC);
						Serial.print("\t"); 
						Serial.println(fac, 2);
 						tp_show_info(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//??��o?��oy?Y   
						delay(1000);
						Tft.lcd_fill_rect(96, 240, 24, 16, WHITE);
						tp_draw_touch_point(LCD_WIDTH - 20, LCD_HEIGHT - 20, WHITE);
						tp_draw_touch_point(20, 20, RED);
						continue;
					}

					tem1=abs((int16_t)(pos_temp[0][0]-pos_temp[2][0]));//x1-x3
					tem2=abs((int16_t)(pos_temp[0][1]-pos_temp[2][1]));//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					tem1+=tem2;
					d1=sqrt(tem1);//

					tem1=abs((int16_t)(pos_temp[1][0]-pos_temp[3][0]));//x2-x4
					tem2=abs((int16_t)(pos_temp[1][1]-pos_temp[3][1]));//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					tem1+=tem2;
					d2=sqrt(tem1);//
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05) {
						cnt=0;
						Serial.print(d1, DEC);
						Serial.print("\t"); 
						Serial.print(d2, DEC);
						Serial.print("\t"); 
						Serial.println(fac, 2);
 						tp_show_info(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//??��o?��oy?Y   
						delay(1000);
						Tft.lcd_fill_rect(96, 240, 24, 16, WHITE);
						tp_draw_touch_point(LCD_WIDTH - 20, LCD_HEIGHT - 20, WHITE);
						tp_draw_touch_point(20, 20, RED);
						continue;
					}//
								   
					tem1=abs((int16_t)(pos_temp[1][0]-pos_temp[2][0]));//x1-x3
					tem2=abs((int16_t)(pos_temp[1][1]-pos_temp[2][1]));//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					tem1+=tem2;
					d1=sqrt(tem1);//

					tem1=abs((int16_t)(pos_temp[0][0]-pos_temp[3][0]));//x2-x4
					tem2=abs((int16_t)(pos_temp[0][1]-pos_temp[3][1]));//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					tem1+=tem2;
					d2=sqrt(tem1);//
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05) {
						cnt=0;	
						Serial.print(d1, DEC);
						Serial.print("\t"); 
						Serial.print(d2, DEC);
						Serial.print("\t"); 
						Serial.println(fac, 2);
 						tp_show_info(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//??��o?��oy?Y   
						delay(1000);
						Tft.lcd_fill_rect(96, 240, 24, 16, WHITE);
						tp_draw_touch_point(LCD_WIDTH - 20, LCD_HEIGHT - 20, WHITE);
						tp_draw_touch_point(20, 20, RED);
						continue;
					}

					s_tTouch.fXfac = (float)(LCD_WIDTH - 40) / (int16_t)(pos_temp[1][0] - pos_temp[0][0]);	
					s_tTouch.iXoff = (LCD_WIDTH - s_tTouch.fXfac * (pos_temp[1][0] + pos_temp[0][0])) / 2;

					s_tTouch.fYfac = (float)(LCD_HEIGHT - 40) / (int16_t)(pos_temp[2][1] - pos_temp[0][1]);	  
					s_tTouch.iYoff = (LCD_HEIGHT - s_tTouch.fYfac * (pos_temp[2][1] + pos_temp[0][1])) / 2;

					Serial.println(s_tTouch.fXfac);
					Serial.println(s_tTouch.iXoff);
					Serial.println(s_tTouch.fYfac);
					Serial.println(s_tTouch.iYoff);


					
					if(abs(s_tTouch.fXfac) > 2 || abs(s_tTouch.fYfac) > 2) {
						cnt=0;
 				    	tp_draw_touch_point(LCD_WIDTH - 20, LCD_HEIGHT - 20, WHITE);
						tp_draw_touch_point(20, 20, RED);								
						Tft.lcd_display_string(40, 26, (const uint8_t *)"TP Need readjust!", 16, RED);
						continue;
					}
					Tft.lcd_clear_screen(WHITE);
					Tft.lcd_display_string(35, 110, (const uint8_t *)"Touch Screen Adjust OK!", 16, BLUE);
					delay(1000); 
 					Tft.lcd_clear_screen(WHITE);  
					return;				 
			}
		}
		delay(10);
		if (++ hwTimeout >= 1000) {
			break;
		}
 	}
}

void TP::tp_adjust_def(void)
{

					s_tTouch.fXfac = -0.14;		
					s_tTouch.iXoff = 260;
					s_tTouch.fYfac = -0.18;	  
					s_tTouch.iYoff = 350;
}

void TP::tp_dialog(void)
{
	Tft.lcd_clear_screen(BLACK);
	Tft.lcd_display_string(LCD_WIDTH - 40, 0, (const uint8_t *)"CLEAR", 16, color);
	Tft.lcd_fill_rect(0,0,15,15,RED);
	Tft.lcd_fill_rect(15,0,15,15,BLUE);
	Tft.lcd_fill_rect(31,0,15,15,GREEN);
	Tft.lcd_fill_rect(47,0,15,15,YELLOW);
	Tft.lcd_fill_rect(63,0,15,15,WHITE);
	
}



void TP::tp_draw_board(void)
{

	tp_scan(0);
	if (s_tTouch.chStatus & TP_PRESS_DOWN) {
		if (s_tTouch.hwXpos < LCD_WIDTH && s_tTouch.hwYpos < LCD_HEIGHT) {
			if (s_tTouch.hwXpos > (LCD_WIDTH - 40) && s_tTouch.hwYpos < 16) 
				{
				tp_dialog();
				} 
			 
			if (s_tTouch.hwXpos > 0 && s_tTouch.hwXpos < 15 && s_tTouch.hwYpos > 0 && s_tTouch.hwYpos < 15) 
				{
				color=RED;
				Tft.lcd_display_string(LCD_WIDTH - 40, 0, (const uint8_t *)"CLEAR", 16, color);
				} 
			if (s_tTouch.hwXpos > 15 && s_tTouch.hwXpos < 30 && s_tTouch.hwYpos > 0 && s_tTouch.hwYpos < 15) 
				{
				color=BLUE;
				Tft.lcd_display_string(LCD_WIDTH - 40, 0, (const uint8_t *)"CLEAR", 16, color);
				} 
			if (s_tTouch.hwXpos > 31 && s_tTouch.hwXpos < 46 && s_tTouch.hwYpos > 0 && s_tTouch.hwYpos < 15) 
				{
				color=GREEN;
				Tft.lcd_display_string(LCD_WIDTH - 40, 0, (const uint8_t *)"CLEAR", 16, color);
				} 
			if (s_tTouch.hwXpos > 47 && s_tTouch.hwXpos < 62 && s_tTouch.hwYpos > 0 && s_tTouch.hwYpos < 15) 
				{
				color=YELLOW;
				Tft.lcd_display_string(LCD_WIDTH - 40, 0, (const uint8_t *)"CLEAR", 16, color);
				} 
			if (s_tTouch.hwXpos > 63 && s_tTouch.hwXpos < 78 && s_tTouch.hwYpos > 0 && s_tTouch.hwYpos < 15) 
				{
				color=WHITE;
				Tft.lcd_display_string(LCD_WIDTH - 40, 0, (const uint8_t *)"CLEAR", 16, color);
				} 
			else	
				{	
					tp_draw_big_point(s_tTouch.hwXpos, s_tTouch.hwYpos, color);
					
				}
		}
	}
}

int TP::tp_get_x() {
	tp_scan(0);
	if (s_tTouch.chStatus & TP_PRESS_DOWN) {
		return s_tTouch.hwXpos;
	}
	else return 1000;
}

int TP::tp_get_y() {
	tp_scan(0);
	if (s_tTouch.chStatus & TP_PRESS_DOWN) {
		return s_tTouch.hwYpos;
	}
	else return 1000;
}

void TP::tp_draw_button(int count, int lenght, char* text, int color)
{
int bx[8]={0,120,0,120,0,120,0,120};
int by[8]={161,161,201,201,241,241,281,281};

	Tft.lcd_draw_rect(bx[count-1],by[count-1],118,38,color); //poga nr1
    int temp_pan=(118-(lenght*8))/2;
    // Serial.println(temp_pan);
    // Serial.println(lenght);
    Tft.lcd_display_string(bx[count-1]+temp_pan,by[count-1]+11,text,FONT_1608,color);

}

int TP::tp_is_button()
{
	int poga;
	tp_scan(0);
		if (s_tTouch.chStatus & TP_PRESS_DOWN)

		{
			if (s_tTouch.hwYpos>159 && s_tTouch.hwYpos<319)
			{
				if (s_tTouch.hwXpos>0 && s_tTouch.hwXpos <120)
				{

					poga=( ( (s_tTouch.hwYpos / 40) -3) + ((s_tTouch.hwYpos / 40) -4));		
					if (poga>0)
					{
						return poga;
					}
					
				}
				if (s_tTouch.hwXpos>120 && s_tTouch.hwXpos <240 )
				{
					poga=( ( (s_tTouch.hwYpos / 40) -3) + ((s_tTouch.hwYpos / 40) -4))+1;
					if (poga>0)
					{
						return poga;
					}
				}
		}	}
		else return 0;

}

TP Tp = TP();


