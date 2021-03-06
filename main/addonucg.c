/******************************************************************************
 * 
 * Copyright 2017 karawin (http://www.karawin.fr)
 *
*******************************************************************************/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_heap_trace.h"
#include "gpio.h"
#include "addon.h"
#include "ucg_esp32_hal.h"
#include "app_main.h"
#include <time.h>
#include "esp_log.h"
#include "logo.h"
#include "interface.h"
#include "eeprom.h"

#define TAG  "addonucg"

extern const ucg_fntpgm_uint8_t ucg_font_crox1c[] UCG_FONT_SECTION("ucg_font_crox1c");
extern const ucg_fntpgm_uint8_t ucg_font_crox1cb[] UCG_FONT_SECTION("ucg_font_crox1cb");
extern const ucg_fntpgm_uint8_t ucg_font_crox1h[] UCG_FONT_SECTION("ucg_font_crox1h");
extern const ucg_fntpgm_uint8_t ucg_font_crox1hb[] UCG_FONT_SECTION("ucg_font_crox1hb");
extern const ucg_fntpgm_uint8_t ucg_font_crox1t[] UCG_FONT_SECTION("ucg_font_crox1t");
extern const ucg_fntpgm_uint8_t ucg_font_crox1b[] UCG_FONT_SECTION("ucg_font_crox1tb");
extern const ucg_fntpgm_uint8_t ucg_font_crox2c[] UCG_FONT_SECTION("ucg_font_crox2c");
extern const ucg_fntpgm_uint8_t ucg_font_crox2cb[] UCG_FONT_SECTION("ucg_font_crox2cb");
extern const ucg_fntpgm_uint8_t ucg_font_crox2h[] UCG_FONT_SECTION("ucg_font_crox2h");
extern const ucg_fntpgm_uint8_t ucg_font_crox2hb[] UCG_FONT_SECTION("ucg_font_crox2hb");
extern const ucg_fntpgm_uint8_t ucg_font_crox2t[] UCG_FONT_SECTION("ucg_font_crox2t");
extern const ucg_fntpgm_uint8_t ucg_font_crox2b[] UCG_FONT_SECTION("ucg_font_crox2tb");
extern const ucg_fntpgm_uint8_t ucg_font_crox3c[] UCG_FONT_SECTION("ucg_font_crox3c");
extern const ucg_fntpgm_uint8_t ucg_font_crox3cb[] UCG_FONT_SECTION("ucg_font_crox3cb");
extern const ucg_fntpgm_uint8_t ucg_font_crox3h[] UCG_FONT_SECTION("ucg_font_crox3h");
extern const ucg_fntpgm_uint8_t ucg_font_crox3hb[] UCG_FONT_SECTION("ucg_font_crox3hb");
extern const ucg_fntpgm_uint8_t ucg_font_crox3t[] UCG_FONT_SECTION("ucg_font_crox3t");
extern const ucg_fntpgm_uint8_t ucg_font_crox3b[] UCG_FONT_SECTION("ucg_font_crox3tb");
extern const ucg_fntpgm_uint8_t ucg_font_crox4h[] UCG_FONT_SECTION("ucg_font_crox4h");
extern const ucg_fntpgm_uint8_t ucg_font_crox4hb[] UCG_FONT_SECTION("ucg_font_crox4hb");
extern const ucg_fntpgm_uint8_t ucg_font_crox4t[] UCG_FONT_SECTION("ucg_font_crox4t");
extern const ucg_fntpgm_uint8_t ucg_font_crox4tb[] UCG_FONT_SECTION("ucg_font_crox4tb");
extern const ucg_fntpgm_uint8_t ucg_font_crox5h[] UCG_FONT_SECTION("ucg_font_crox5h");
extern const ucg_fntpgm_uint8_t ucg_font_crox5hb[] UCG_FONT_SECTION("ucg_font_crox5hb");
extern const ucg_fntpgm_uint8_t ucg_font_crox5t[] UCG_FONT_SECTION("ucg_font_crox5t");
extern const ucg_fntpgm_uint8_t ucg_font_crox5tb[] UCG_FONT_SECTION("ucg_font_crox5tb");


#define ucg_SetColori(a,b,c,d) ucg_SetColor(a,0,b,c,d)

// TOP Background & str  COLOR
#define CTBACK 50,50,120
#define CTTFONT 250,250,0
// Body font color
#define CBODY 110,255,110

#define CBLACK 0,0,0
#define CWHITE 255,255,255
#define CRED 255,10,10
 

// nams <--> num of line
#define STATIONNAME 0
#define STATION1  1
#define STATION2  2
#define IP        3
#define GENRE     2
#define TITLE1    3
#define TITLE11   4
#define TITLE2    5
#define TITLE21   6
#define VOLUME    7
#define TIME      8

#define BUFLEN  256
#define LINES	9

static uint16_t y ;		//Height of a line
static uint16_t yy;		//Height of screen
static uint16_t x ;		//Width
static uint16_t z ;		// an internal offset for y
static uint16_t HHeader= 40;

//static struct tm *dt;
static char strsec[30]; 
static uint16_t volume;

static char station[BUFLEN]; //received station
static char title[BUFLEN];	// received title
static char nameset[BUFLEN]; // the local name of the station

static char* lline[LINES] ; // array of ptr of n lines 
static uint8_t  iline[LINES] ; //array of index for scrolling
static uint8_t  tline[LINES] ;
static uint8_t  mline[LINES] ; // mark to display

static char nameNum[5] ; // the number of the current station
static char genre[BUFLEN/2]; // the local name of the station

static char TTitleStr[15];
static char TTimeStr[15];

static bool charset = false;  // latin or cyrillic

////////////////////////////////////////
typedef enum sizefont  {small, text,middle,large} sizefont;
void setfont(sizefont size)
{
//	printf("setfont charset: %d, size: %d, x: %d\n",charset,size,x);
	switch(size)
	{
		case small:
		switch(x)
		{
			case 320:
			ucg_SetFont(&ucg,ucg_font_6x13_mf);
			break;
			case 128:
			ucg_SetFont(&ucg,ucg_font_4x6_mf);
			break;
			case 96:
			ucg_SetFont(&ucg,ucg_font_u8glib_4_hf);
			break;
			case 132:
			default: // 160
			ucg_SetFont(&ucg,ucg_font_5x8_mf);
			;
		}
		break;

		case text:
		switch(x)
		{
			case 320:
			charset?ucg_SetFont(&ucg,ucg_font_crox5h ):ucg_SetFont(&ucg,ucg_font_inr16_mf ) ;
			break;
			case 128:
			charset?ucg_SetFont(&ucg,ucg_font_crox1c ):ucg_SetFont(&ucg,ucg_font_5x7_mf) ;
			break;
			case 132:
			charset?ucg_SetFont(&ucg,ucg_font_crox1c ):ucg_SetFont(&ucg,ucg_font_5x7_mf) ;
			break;
			case 96:
			charset?ucg_SetFont(&ucg,ucg_font_crox1c ):ucg_SetFont(&ucg,ucg_font_4x6_mf) ;
			break;
			default: // 160
			charset?ucg_SetFont(&ucg,ucg_font_crox1c ):ucg_SetFont(&ucg,ucg_font_6x13_mf) ;
			;
		}
		break;

		case middle:
		switch(x)
		{
			case 320:
			charset?ucg_SetFont(&ucg,ucg_font_crox5h ):ucg_SetFont(&ucg,ucg_font_inr33_mf);
			break;
			case 128:
			charset?ucg_SetFont(&ucg,ucg_font_crox3c ):ucg_SetFont(&ucg,ucg_font_7x14_mf);
			break;
			case 96:
			charset?ucg_SetFont(&ucg,ucg_font_crox2h ):ucg_SetFont(&ucg,ucg_font_6x12_mf);
			break;
			case 132:
			default: // 160
			charset?ucg_SetFont(&ucg,ucg_font_crox3c ):ucg_SetFont(&ucg,ucg_font_fur14_tf);
			
			;
		}
		break;
		case large:
		switch(x)
		{
			case 320:
			ucg_SetFont(&ucg,ucg_font_inr53_mf); 
			break;
			case 128:
			ucg_SetFont(&ucg,ucg_font_helvR12_hf); 
			break;
			case 96:
			ucg_SetFont(&ucg,ucg_font_helvR12_hf); 
			break;
			case 132:
			default: // 160
			ucg_SetFont(&ucg,ucg_font_inr38_mr); 
			//ucg_SetFont(&ucg, ucg_font_helvB18_tf);
			;
		}
		break;
		default:
		printf("Default for size %d\n",size);
	}
}


////////////////////////////////////////
char* getNameNumUcg()
{
	return nameNum;
}	

void setVolumeUcg(uint16_t vol){ volume = vol;}

////////////////////////////////////////
// Clear all buffers and indexes
void clearAllUcg()
{
      title[0] = 0;
      station[0]=0;
    for (int i=1;i<LINES;i++) {lline[i] = NULL;iline[i] = 0;tline[i] = 0;;mline[i]=1;}
}
////////////////////////////////////////
void cleartitleUcg(uint8_t froml)
{
     title[0] = 0;
     for (int i = froml;i<LINES;i++)  // clear lines
     {
		lline[i] = NULL;
		iline[i] = 0;
		tline[i] = 0;
		mline[i] = 1;
     }  
}

//Thanks to Max
void ucEraseSlashes(char * str) {
	//Symbols: \" \' \\ \? \/
	char * sym = str, * sym1;
	if (str != NULL) {
		while (*sym != 0) {
			if (*sym == 0x5c) {
				sym1 = sym + 1;
				if (*sym1 == 0x22 || *sym1 == 0x27 || *sym1 == 0x5c || *sym1 == 0x3f || *sym1 == 0x2f) {
					*sym = 0x1f; //Erase \ to non-printable symbol
					sym++;
				}	
			} 
			sym++;
		}
	} 	
}
//-Max

// non linear cyrillic conversion
struct _utf8To1251_t
{
  uint16_t utf8;
  uint8_t c1251;

};
typedef struct _utf8To1251_t utf8To1251_t;
#define UTF8TO1251	30
utf8To1251_t utf8To1251[UTF8TO1251] = {{0x401,0xa8},{0x402,0x80},{0x403,0x81},{0x404,0xaa},{0x405,0xbd},{0x406,0x49/*0xb2*/},{0x407,0xaf},{0x408,0xa3},
									   {0x409,0x8a},{0x40a,0x8c},{0x40b,0x8e},{0x40c,0x8d},{0x40e,0xa1},{0x40f,0x8f},{0x452,0x90},{0x451,0xb8},
									   {0x453,0x83},{0x454,0xba},{0x455,0xbe},{0x456,0x69/*0xb3*/},{0x457,0xbf},{0x458,0x6a/*0xbc*/},{0x459,0x9a},{0x45a,0x9c},
									   {0x45b,0x9e},{0x45c,0x9d},{0x45f,0x9f},{0x490,0xa5},{0x491,0xb4},
									   {0,0}};
uint8_t to1251(uint16_t utf8)
{
	int i;
	if (utf8 > 0x491) return 0x1f;
	for (i = 0; i<UTF8TO1251;i++)
	{
		if (utf8 == utf8To1251[i].utf8)
		{
//			printf("to1251: utf8: %x, ret: %x\n",utf8,utf8To1251[i].c1251);
			return utf8To1251[i].c1251;
		}
	}
	
//	printf("to1251: utf8: %x, ret: %x\n",utf8,(utf8 - 0x350)& 0xff);
	return ((utf8 - 0x350)& 0xff );
}


////////////////////////////////////////
uint16_t UtoC(uint8_t high,uint8_t low)
{
	uint16_t res = (( high<<6)  |( low & 0x3F )) & 0x7FF;
	return(res);
}

void removeUtf8(char *characters)
{
  int Rindex = 0;
  uint16_t utf8;
  ESP_LOGV(TAG,"removeUtf8 in : %s",characters);
  ucEraseSlashes(characters) ; 
  while (characters[Rindex])
  {
    if ((characters[Rindex] >= 0xc2)&&(characters[Rindex] <=0xc3)) // only 0 to FF ascii char
    {
		utf8 = UtoC(characters[Rindex],characters[Rindex+1]) ; // the utf8
		characters[Rindex+1] =  (uint8_t)utf8 &0xff;
		if (utf8>= 0x100) characters[Rindex+1] = 0x1f; //Erase to non-printable symbol
		int sind = Rindex+1;
		while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
		characters[sind-1] = 0; 
    }
    if ((characters[Rindex] >= 0xd0)&&(characters[Rindex] <= 0xd3)) // only 0 to FF ascii char
    {	
		utf8 = UtoC(characters[Rindex],characters[Rindex+1]) ; // the utf8
		characters[Rindex+1] = to1251(utf8);
		int sind = Rindex+1;
		while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
		characters[sind-1] = 0;
		charset = true;
	}
    Rindex++;
  }

  ESP_LOGV(TAG,"removeUtf8 out: %s",characters);
  
}

// Mark the lines to draw
void markDrawUcg(int i)
{
  mline[i] = 1;
}


////////////////////////////////////////
// scroll each line
void scrollUcg()
{
int16_t len;
setfont(text);
	for (int i = 0;i < LINES;i++)
	{  
		if (lline[i] != NULL)
		{	
			if (tline[i]>0) 
			{
				len = (i==0)? ucg_GetStrWidth(&ucg,nameNum)+ucg_GetStrWidth(&ucg,lline[i]):ucg_GetStrWidth(&ucg,lline[i]);
				if ((tline[i] == 4) && (len > x)) 
				{
					iline[i]= 0;
					markDrawUcg(i);//draw(i);
				}
				tline[i]--;		 
			} 
			else
			{
				len = (i==0)? ucg_GetStrWidth(&ucg,nameNum)+ucg_GetStrWidth(&ucg,lline[i]+iline[i]):ucg_GetStrWidth(&ucg,lline[i]+iline[i]);
				if (len > x)
				{      
					iline[i] += x/ucg_GetStrWidth(&ucg,"MM");//x/6;
					len = iline[i];
					while ((*(lline[i]+iline[i])!=' ')&&(*(lline[i]+iline[i])!='-')&&(iline[i]!= 0))iline[i]--;
					if (iline[i]==0) iline[i]=len;     
					markDrawUcg(i); //draw(i);
				}
				else 
					{tline[i] = 6;}
			}
		}
	}
}


//////////////////////////
// set color of font per line
void setColor(int i)
{
        switch(i){
          case STATIONNAME: ucg_SetColori(&ucg,0,0,0); break;
          case STATION1: ucg_SetColori(&ucg,255,255,255); break;
          case STATION2: ucg_SetColori(&ucg,255,200,200);  break;
          case TITLE1:
          case TITLE11: ucg_SetColori(&ucg,255,255,0);  break;
          case TITLE2:
          case TITLE21: ucg_SetColori(&ucg,0,255,255); break; 
          case VOLUME:  ucg_SetColori(&ucg,200,200,255); break; 
          default:ucg_SetColor(&ucg,0,CBODY);  
        }  
}

////////////////////
// draw one line
void draw(int i)
{
	uint16_t len,xpos,yyy; 
	
    if ( mline[i]) mline[i] =0;
    if (i >=3) z = y/2 ; else z = 0;
    switch (i) {
        case STATIONNAME:
		setfont(text);
        ucg_SetColori(&ucg,255,255,255);  
        ucg_DrawBox(&ucg,0,0,x,y-1/*-ucg_GetFontDescent(&ucg)*/);  
        ucg_SetColori(&ucg,0,0,0);  
		if (lline[i] != NULL)
		{
			if (nameNum[0] ==0)  ucg_DrawString(&ucg,1,1,0,lline[i]+iline[i]);
			else 
			{
			ucg_DrawString(&ucg,1,1,0,nameNum);
			ucg_DrawString(&ucg,ucg_GetStrWidth(&ucg,nameNum)-2,1,0,lline[i]+iline[i]);
			}
		}
        break;
        case VOLUME:
 		if ((yy > 64)||(lline[TITLE21] == NULL)||(strlen(lline[TITLE21]) ==0))
		{
          ucg_SetColori(&ucg,0,0,200); 
		  if (yy <= 64)
		  {
			ucg_DrawFrame(&ucg,0,yy-10,x/3,8); 
			ucg_SetColori(&ucg,255,0,0); 
			ucg_DrawBox(&ucg,1,yy-9,((uint16_t)(x/3*volume)/255),6); 
		  }
		  else
		  {
			ucg_DrawFrame(&ucg,0,yy-10,x/2,8); 
			ucg_SetColori(&ucg,255,0,0); 
			ucg_DrawBox(&ucg,1,yy-9,((uint16_t)(x/2*volume)/255),6); 
		  }
		}		  
        break;
        case TIME:
 		if ((yy > 64)||(lline[TITLE21] == NULL)||(strlen(lline[TITLE21]) ==0))
		{
		  setfont(small);
          len = ucg_GetStrWidth(&ucg,strsec);
          ucg_SetColori(&ucg,250,250,255); 
          ucg_SetColor(&ucg,1,CBLACK); 
          ucg_SetFontMode(&ucg,UCG_FONT_MODE_SOLID);
		  if (yy <= 64)
		  {
			xpos = (5*x/8)-(len/2);
			yyy = yy -10;
			ucg_DrawString(&ucg,xpos,yyy,0,strsec); 
		  } else
		  {
			xpos = (3*x/4)-(len/2);
			yyy = yy -10;
			ucg_DrawString(&ucg,xpos,yyy,0,strsec); 
		  }			  
          ucg_SetFontMode(&ucg,UCG_FONT_MODE_TRANSPARENT);
		}
        break;
        default:
          ucg_SetColori(&ucg,0,0,0); 
          ucg_DrawBox(&ucg,0,y*i+z,x,y-ucg_GetFontDescent(&ucg)); 
          setColor(i);
          if (lline[i] != NULL) ucg_DrawString(&ucg,0,y*i+z+1,0,lline[i]+iline[i]);                
   }      
}


////////////////////////////////////////
// draw the full screen
void drawLinesUcg()
{
	setfont(text);
    for (int i=0;i<LINES;i++)
    {
        if (mline[i]) draw(i); 
    }
}



////////////////////////////////////////
// draw all
void drawFrameUcg(uint8_t mTscreen,struct tm *dt)
{
//printf("drawFrameUcg, mTscreen: %d\n",mTscreen);
int i;
    switch (mTscreen){
    case 1: 
		ucg_ClearScreen(&ucg);
		TTitleStr[0] = 0;   
		setfont(text);
		ucg_SetColor(&ucg,0,255,255,0);  
		ucg_SetColor(&ucg,1,0,255,255);  
		ucg_DrawGradientLine(&ucg,0,(4*y) - (y/2) -4,x,0);
		ucg_SetColor(&ucg,0,CBLACK);  
		ucg_DrawBox(&ucg,0,0,x-1,15);  
		for (i=0;i<LINES;i++) draw(i);
		// no break
	case 2:	
		if (getDdmm())
			sprintf(strsec,"%02d-%02d  %02d:%02d:%02d",dt->tm_mday,dt->tm_mon+1,dt->tm_hour, dt->tm_min,dt->tm_sec);
		else
			sprintf(strsec,"%02d-%02d  %02d:%02d:%02d",dt->tm_mon+1,dt->tm_mday,dt->tm_hour, dt->tm_min,dt->tm_sec);
		markDrawUcg(TIME);
		drawLinesUcg();
		break;
	 default:;
	}
	
	//screenBottomUcg();    
}


//////////////////////////
void drawTTitleUcg(char* ttitle)
{ 

	if (strcmp(ttitle,TTitleStr) != 0)
	{
		setfont(middle);
		uint16_t xxx = (x/2)-(ucg_GetStrWidth(&ucg,ttitle)/2);
		ucg_SetColor(&ucg,0,CTBACK);  
		ucg_DrawBox(&ucg,0,0,x,HHeader); 
		ucg_SetColor(&ucg,0,CTTFONT);  
		ucg_DrawString(&ucg,xxx,(HHeader-ucg_GetFontAscent(&ucg))>>1,0,ttitle);
		strcpy(TTitleStr,ttitle);
	}
}
//////////////////////////
void drawNumberUcg(uint8_t mTscreen,char* irStr)
{
  uint16_t xxx ;
  char ststr[] = {"Number"};
    switch (mTscreen){
      case 1:     
		TTitleStr[0] = 0;   
        drawTTitleUcg(ststr);   
      // no break
      case 2:  
        xxx = (x/2)-(ucg_GetStrWidth(&ucg,irStr)/2); 
        ucg_SetColor(&ucg,0,CBLACK);  
        ucg_DrawBox(&ucg,0,HHeader,x,yy);     
        setfont(large);
        ucg_SetColor(&ucg,0,CBODY);  
        ucg_DrawString(&ucg,xxx,yy/3,0, irStr);
        break;
      default:; 
    }  

//  screenBottomUcg();  
}
//////////////////////////
void drawStationUcg(uint8_t mTscreen,char* snum,char* ddot)
{
	
  char ststr[] = {"Station"};
  int16_t len;
  bool scharset;
    switch (mTscreen){
      case 1:  
		TTitleStr[0] = 0;        
        drawTTitleUcg(ststr);
      // no break
      case 2:   
        ucg_SetColor(&ucg,0,CBLACK); 
        ucg_DrawBox(&ucg,0,HHeader,x,yy);     
 //       setfont(middle);
        ucg_SetColor(&ucg,0,CBODY);
//        ddot = strstr(sline,":");
        if (ddot != NULL)
        {
		  scharset = charset;
		  charset = false;
		  removeUtf8(ddot);
          ucg_DrawString(&ucg,(x/2)-(ucg_GetStrWidth(&ucg,snum)/2),yy/3,0,snum);
          len = (x/2)-(ucg_GetStrWidth(&ucg,ddot)/2);
          if (len <0) len = 0;
          ucg_DrawString(&ucg,len,yy/3 + ucg_GetFontAscent(&ucg)+y,0, ddot);
		  charset = scharset;
        }
        break;
      default:; 
    } 	

//  screenBottomUcg(); 	
}


void drawVolumeUcg(uint8_t mTscreen,char* aVolume)
{
  char vlstr[] = {"Volume"};
  volume = atoi(aVolume);
    switch (mTscreen){
      case 1: 
		ucg_ClearScreen(&ucg);
		TTitleStr[0] = 0;
        drawTTitleUcg(vlstr) ;		
/*        ucg_SetColor(&ucg,0,CBLACK);  
        ucg_DrawBox(&ucg,0,HHeader,x,yy);     
        ucg_SetColor(&ucg,0,CBODY);   */     // no break
      case 2:
//        ucg_SetFont(&ucg,ucg_font_inr49_tf);
        setfont(large); 
        uint16_t xxx;
        xxx = (x/2)-(ucg_GetStrWidth(&ucg,aVolume)/2);
        ucg_SetColor(&ucg,0,CBLACK);  
		//ucg_SetFontMode(&ucg,UCG_FONT_MODE_SOLID); 
//        ucg_DrawBox(&ucg,0,HHeader,x,yy);     
        ucg_DrawBox(&ucg,0,yy/3,x,ucg_GetFontAscent(&ucg)+2);     
        ucg_SetColor(&ucg,0,CBODY);  
        ucg_DrawString(&ucg,xxx,yy/3,0,aVolume); 
		//ucg_SetFontMode(&ucg,UCG_FONT_MODE_TRANSPARENT);
        break;
      default:; 
    }

//  screenBottomUcg(); 
}

static void drawSecond(struct tm *dt,unsigned timein)
{
  static unsigned insec;
  if (insec != timein)
  {
  char strseco[3]; 
  uint16_t len;
  sprintf(strseco,":%02d",dt->tm_sec);
  setfont(text);
  len = ucg_GetStrWidth(&ucg,"xxx");

  ucg_SetColor(&ucg,1,CBLACK); 
  ucg_SetFontMode(&ucg,UCG_FONT_MODE_SOLID); 
  ucg_SetColor(&ucg,0,CBODY);
  ucg_DrawString(&ucg,x-len-8,yy-18,0,strseco); 
  ucg_SetFontMode(&ucg,UCG_FONT_MODE_TRANSPARENT);
  insec = timein; //to avoid redisplay
  }    
}

void drawTimeUcg(uint8_t mTscreen,struct tm *dt,unsigned timein)
{
  char strdate[23];
  char strtime[20];
    sprintf(strtime,"%02d:%02d", dt->tm_hour, dt->tm_min);
    switch (mTscreen){
      case 1:
		setfont(text);
		sprintf(strdate,"IP: %s", getIp());
		ucg_ClearScreen(&ucg);
        ucg_SetColor(&ucg,0,CRED);  
		TTitleStr[0] = 0;
		TTimeStr[0] = 0;
//        ucg_SetColor(&ucg,0,CBLACK);  
//        ucg_DrawBox(&ucg,0,HHeader,x,yy);     		
        // draw ip
        //ucg_SetFont(&ucg,ucg_font_6x13_tf);
        ucg_DrawString(&ucg,4,yy-18,0,strdate);		
      case 2:
	    if (getDdmm())
			sprintf(strdate,"%02d-%02d-%04d", dt->tm_mday, dt->tm_mon+1,  dt->tm_year+1900);
	    else
			sprintf(strdate,"%02d-%02d-%04d", dt->tm_mon+1, dt->tm_mday, dt->tm_year+1900);
		drawTTitleUcg(strdate);
		if (strcmp(TTimeStr,strtime)!= 0)
		{	
			//ucg_SetFont(&ucg,ucg_font_inr38_mf); 
			setfont(large);
			ucg_SetColor(&ucg,0,CBODY);		
			ucg_SetFontMode(&ucg,UCG_FONT_MODE_SOLID); 
			ucg_DrawString(&ucg,(x/2)-(ucg_GetStrWidth(&ucg,strtime)/2),yy/3,0,strtime); 
			strcpy(TTimeStr,strtime);
			ucg_SetFontMode(&ucg,UCG_FONT_MODE_TRANSPARENT);
		}

		break;
      default:;
    }
	drawSecond(dt,timein);;     	
}


////////////////////////////////////////
void separatorUcg(char* from)
{
    char* interp;
//    len = strlen(from);
    //ucg_SetFont(&ucg,ucg_font_6x13_tf);
	setfont(text);
    while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
    while ((from[0] == ' ') ){ strcpy( from,from+1); }
    interp=strstr(from," - ");
	if (from == nameset) {/*lline[0] = nameset;*/lline[1] = NULL;lline[2] = NULL;return;}
	if (interp != NULL)
	{
	  from[interp-from]= 0;
	  lline[(from==station)?STATION1:TITLE1] = from;
	  lline[(from==station)?STATION2:TITLE2] = interp+3;
    mline[(from==station)?STATION1:TITLE1]=1;
    mline[(from==station)?STATION2:TITLE2]=1;
	} else
	{
	  lline[(from==station)?STATION1:TITLE1] = from;
    mline[(from==station)?STATION1:TITLE1]=1;
	}

// 2 lines for Title
 if ((lline[TITLE1]!= NULL)&&(from == title)&&(ucg_GetStrWidth(&ucg,lline[TITLE1]) > x))
 {
    int idx = strlen(lline[TITLE1]);
    *(lline[TITLE1]+idx) = ' ';
    *(lline[TITLE1]+idx+1) = 0;
    while ((ucg_GetStrWidth(&ucg,lline[TITLE1]) > x)&&(idx !=0))
    {
      *(lline[TITLE1]+idx--)= ' ';
      while ((*(lline[TITLE1]+idx)!= ' ')&&(idx !=0)) idx--;
      if (idx != 0) *(lline[TITLE1]+idx)= 0;
    }
    lline[TITLE11] = lline[TITLE1]+idx+1;
    mline[TITLE11]=1; 
 }
 
 if ((lline[TITLE2]!= NULL)&&(from == title)&&(ucg_GetStrWidth(&ucg,lline[TITLE2]) > x))
 {
    int idx = strlen(lline[TITLE2]);
    *(lline[TITLE2]+idx) = ' ';
    *(lline[TITLE2]+idx+1) = 0;
    while ((ucg_GetStrWidth(&ucg,lline[TITLE2]) > x)&&(idx !=0))
    {
      *(lline[TITLE2]+idx--)= ' ';
      while ((*(lline[TITLE2]+idx)!= ' ')&&(idx !=0)) idx--;
      if (idx != 0) *(lline[TITLE2]+idx)= 0;
    }
    lline[TITLE21] = lline[TITLE2]+idx+1;
    mline[TITLE21]=1; 
 }
}

//cli.meta
void metaUcg(char* ici)
{
     cleartitleUcg(3); 
     strcpy(title,ici+7);    
	 removeUtf8(title);
     separatorUcg(title); 	
}

//cli.icy4
void icy4Ucg(char* ici)
{
	 strcpy(genre,ici+7);
     removeUtf8(genre); 
     lline[2] = genre;
}
//cli.icy0
void icy0Ucg(char* ici)
{
      clearAllUcg();
      if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
	  removeUtf8(station);
      separatorUcg(station);	
}

//cli.stopped or label
void statusUcg(char* label)
{
     cleartitleUcg(3);
     strcpy(title,label);
     lline[TITLE1] = title;	
}
//cli.nameset
void namesetUcg(char* ici)
{
	strcpy(nameset,ici+8);
    ici = strstr(nameset," ");
    if (ici != NULL)
    {
       clearAllUcg();
       strncpy(nameNum,nameset,ici-nameset+1);
       nameNum[ici - nameset+1] = 0; 
	   setFuturNum(atoi(nameNum));     
    }
    strcpy(nameset,nameset+strlen(nameNum));
	charset = false;
	removeUtf8(nameset);
    lline[STATIONNAME] = nameset;
}

// cli.playing
void playingUcg()
{
	if (strcmp(title,"STOPPED") == 0)
    {
        cleartitleUcg(3);
        separatorUcg(title);
    }
}



void lcd_initUcg(uint8_t *lcd_type)
{
	uint8_t rotat = getRotat();
	ESP_LOGI(TAG,"lcd init  type: %d",*lcd_type);
	
		ucg_esp32_hal_t ucg_esp32_hal = UCG_ESP32_HAL_DEFAULT;
		if (*lcd_type & LCD_SPI) // Color SPI
		{
			ucg_esp32_hal.clk   = PIN_NUM_CLK;
			ucg_esp32_hal.mosi  = PIN_NUM_MOSI;
			ucg_esp32_hal.cs    = PIN_LCD_CS;
			ucg_esp32_hal.dc    = PIN_LCD_A0;
			ucg_esp32_hal.reset = PIN_LCD_RST;
		} else //Color I2c
		{
			ucg_esp32_hal.sda  = PIN_I2C_SDA;
			ucg_esp32_hal.scl  = PIN_I2C_SCL;
			ucg_esp32_hal.reset = PIN_LCD_RST;
		}
		
		ucg_esp32_hal_init(ucg_esp32_hal);	
		
	switch (*lcd_type){		
// Color spi
	case LCD_SPI_SSD1351:	
		ucg_Init(&ucg, ucg_dev_ssd1351_18x128x128_ilsoft, ucg_ext_ssd1351_18, ucg_com_hal);
		break;
	case LCD_SPI_ST7735:
		ucg_Init(&ucg, ucg_dev_st7735_18x128x160, ucg_ext_st7735_18, ucg_com_hal);
		break;
	case LCD_SPI_ILI9341:
		ucg_Init(&ucg, ucg_dev_ili9341_18x240x320, ucg_ext_ili9341_18, ucg_com_hal);
		break;
	case LCD_SPI_ILI9163:
		ucg_Init(&ucg, ucg_dev_ili9163_18x128x128, ucg_ext_ili9163_18, ucg_com_hal);
		break;	
	case LCD_SPI_PCF8833:
		ucg_Init(&ucg, ucg_dev_pcf8833_16x132x132, ucg_ext_pcf8833_16, ucg_com_hal);
		break;	
	case LCD_SPI_SSD1331:
		ucg_Init(&ucg, ucg_dev_ssd1331_18x96x64_univision, ucg_ext_ssd1331_18, ucg_com_hal);
		break;	
	case LCD_SPI_SEPS225:
		ucg_Init(&ucg, ucg_dev_seps225_16x128x128_univision, ucg_ext_seps225_16, ucg_com_hal);
		break;	
	default: 
		ESP_LOGE(TAG,"lcd invalid type: %d",*lcd_type);
		return;
	}	
		
		ESP_LOGI(TAG,"lcd init Color type: %d",*lcd_type);
		// define prefered font rendering method (no text will be visibile, if this is missing 
		ucg_SetFontMode(&ucg, UCG_FONT_MODE_TRANSPARENT); 
		ucg_ClearScreen(&ucg);		
		
		
		if (rotat)
			ucg_SetRotate270(&ucg);
		else 
			ucg_SetRotate90(&ucg);
		
/*		switch (*lcd_type)
		{
			case LCD_SPI_ILI9341:
				ucg_SetRotate270(&ucg);
				break;
			case LCD_SPI_SSD1331:
				break;
			default:
			ucg_SetRotate90(&ucg);			
		}
*/		
		
		//ucg_SetFont(&ucg,ucg_font_6x13_tf);
		ucg_SetFontPosTop(&ucg);
		x  = ucg_GetWidth(&ucg);
		
		setfont(text);
		yy = ucg_GetHeight(&ucg);
		if (yy == 64)
			y = - ucg_GetFontDescent(&ucg)+ ucg_GetFontAscent(&ucg)+2 ; //interline
		else
			y = - ucg_GetFontDescent(&ucg)+ ucg_GetFontAscent(&ucg) +3; //interline
		
		HHeader = yy/5;		
		
		printf("X: %d, yy: %d, y: %d\n",x,yy,y);
		z = 0; 
}

