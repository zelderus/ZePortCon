/*
 * ���          :  n3310.h
 *
 * ��������     :  ��� ������������ ���� ��� �������� ������������ LCD �� Nokia 3310, � ����� ��� ��������� ������.
 *                 ���������� �� ���� ��������� ���������� Sylvain Bissonnette � Fandi Gunawan:
 *                 http://www.microsyl.com/index.php/2010/03/24/nokia-lcd-library/
 *                 http://fandigunawan.wordpress.com/2008/06/18/lcd-nokia-3310-pcd8544-driver-in-winavravr-gcc/
 *                 �������� ������� ����� ���������� � ������ ������ ������� � ������ �� Aheir:
 *                 http://radiokot.ru/articles/29/
 *
 * �����        :  Xander Gresolio <xugres@gmail.com>
 * ���-�������� :  http://we.easyelectronics.ru/profile/XANDER/
 *
 * ��������     :  GPL v3.0
 *
 * ����������   :  WinAVR, GCC for AVR platform
 */

#ifndef _N3310_H_
#define _N3310_H_



#include "../../settings.h"






#define ENABLE_BITMAPS		1


// ���������� ������� � ��������
#define LCD_X_RES                  LCD_WIDTH     // ���������� �� �����������
#define LCD_Y_RES                  LCD_HEIGHT    // ���������� �� ���������

//// ��������� ��� ��������� ������ ��������������� �������� LcdBars ( byte data[], byte numbBars, byte width, byte multiplier )
//#define EMPTY_SPACE_BARS           2     // ���������� ����� ����������������
//#define BAR_X                      30    // ���������� x
//#define BAR_Y                      47    // ���������� y

// ������ ���� ( 84 * 48 ) / 8 = 504 �����
#define LCD_CACHE_SIZE             ( ( LCD_X_RES * LCD_Y_RES ) / 8 )

#define FALSE                      0
#define TRUE                       1

// ��� ������������ ��������
#define OK                         0   // ������������ ���������
#define OUT_OF_BORDER              1   // ����� �� ������� �������
#define OK_WITH_WRAP               2   // ������� �� ������ (�������� �������������� ��������� ������� ��� ������ �������� ������)
#define OK_WITH_WRAP_LINE          3   // ������� �� ����� ������� (�������� �������������� ��������� ������� ��� ������ �������� ������)

typedef unsigned char              byte;


#define DEC							10
#define HEX							16
#define OCT							8
#define BIN							2



// ������������
typedef enum
{
    LCD_CMD  = 0,     // �������
    LCD_DATA = 1      // ������

} LcdCmdData;

typedef enum
{
    PIXEL_OFF =  0,   // �������� ������� �������
    PIXEL_ON  =  1,   // �������� ������� �������
    PIXEL_XOR =  2    // ������������� �������

} LcdPixelMode;

typedef enum
{
    FONT_1X = 1,      // ������� ������ ������ 5x7
    FONT_2X = 2       // ����������� ������ ������

} LcdFontSize;



////for extended bitmap function :
//#define NOROT 0
//#define ROTCCW 1
//#define ROT180 2
//#define ROTCW 3
typedef enum
{
	NOROT = 0,
	ROTCCW = 1,
	ROT180 = 2,
	ROTCW = 3
} LcdRotator;
//#define NOFLIP 0
//#define FLIPH 1
//#define FLIPV 2
//#define FLIPVH 3
typedef enum
{
	NOFLIP = 0,
	FLIPH = 1,
	FLIPV = 2,
	FLIPVH = 3
} LcdFlip;




// ��������� �������, ��������� ���������� ������� ������ n3310lcd.c
void LcdInit       ( void );   // �������������
void LcdClear      ( void );   // ������� ������
void LcdUpdate     ( void );   // ����������� ������ � ��� �������
void LcdImage      ( const byte *imageData );   // ��������� �������� �� ������� � Flash ROM
void LcdContrast   ( byte contrast );   // ��������� ������������� �������
byte LcdGotoXYFont ( byte x, byte y );   // ��������� ������� � ������� x,y
byte LcdChr        ( LcdFontSize size, byte ch );   // ����� ������� � ������� �������
byte LcdStr        ( LcdFontSize size, byte dataArray[], bool wrapLine = false, bool wrapDisplay = false );   // ����� ������ ����������� � RAM
byte LcdFStr       ( LcdFontSize size, const byte *dataPtr, bool wrapLine = false, bool wrapDisplay = false );   // ����� ������ ����������� � Flash ROM
byte LcdPixel      ( byte x, byte y, LcdPixelMode mode );   // �����
byte LcdLine       ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode );   // �����
byte LcdCircle     ( byte x, byte y, byte radius, LcdPixelMode mode);   // ����������
byte LcdRect       ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode );   // �������������
int  LcdGetCacheIdx();

byte LcdPrint(int, int = DEC);
byte LcdPrint(unsigned int, int = DEC);
byte LcdPrint(long, int = DEC);
byte LcdPrint(unsigned long, int = DEC);



void LcdDrawFastVLine(int8_t x, int8_t y, int8_t h);
void LcdDrawFastHLine(int8_t x, int8_t y, int8_t w);
bool LcdGetBitmapPixel(const uint8_t* bitmap, uint8_t x, uint8_t y);
unsigned int LcdGetBitmapWidth(const uint8_t* bitmap);
unsigned int LcdGetBitmapHeight(const uint8_t* bitmap);
void LcdDrawBitmap(int8_t x, int8_t y, const uint8_t *bitmap);
void LcdDrawBitmap(int8_t x, int8_t y, const uint8_t *bitmap, LcdRotator rotation, LcdFlip flip);


byte __printNumber(unsigned long n, byte base);



#endif  /*  _N3310_H_ */
