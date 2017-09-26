/*
 * ���          :  n3310.c
 *
 * ��������     :  ��� ������� ��� ������������ LCD �� Nokia 3310, � ����� ��� ��������� ������.
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
 *
 * �������      :
 * ������ 1.0 (06.08.2011)
 * + ������ ������
 * + ��������� ��������� ��������� ������ LCD Nokia 3310
 * + ������ ������� ������������ � ��������� ���� ��������
 * + ������� �������� �������� ��������� ���������� (���������� Windows-1251)
 * + ��������� ������� ��������� ����������� LcdCircle
 * - ���������� ������ � �������� ������������ ��������� ��� ������ ������� ���������
 * - ���������� ������ � ������� LcdSingleBar (�������� ��������� �� y)
 */

#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "n3310.h"
#include "fonts.h"
#include "../../System/math.h"





// ��������� ��������� ������� ��������

static void LcdSend    ( byte data, LcdCmdData cd );
static void Delay      ( void );

// ���������� ����������

// ��� � ��� 84*48 ��� ��� 504 �����
static byte  LcdCache [ LCD_CACHE_SIZE ];

// ����� �� ��������� ���� �������, � ���� �� ����� ��� ����������,
// ����� �������� ��� ������� ���� ��� ��������� ���������. �����
// ����� ���������� ��� ����� ���� ����� ��������� � ��� �������.
static int   LoWaterMark;   // ������ �������
static int   HiWaterMark;   // ������� �������

// ��������� ��� ������ � LcdCache[]
static int   LcdCacheIdx;

// ���� ��������� ����
static byte  UpdateLcd;


int LcdGetCacheIdx()
{
	return LcdCacheIdx;
}


/*
 * ���                   :  LcdInit
 * ��������              :  ���������� ������������� ����� � SPI ��, ����������� LCD
 * ��������(�)           :  ���
 * ������������ �������� :  ���
 */
void LcdInit ( void )
{

    // Pull-up �� ����� ������������ � reset �������
    LCD_PORT |= _BV ( LCD_RST_PIN );

    // ������������� ������ ���� ����� �� �����
    LCD_DDR |= _BV( LCD_RST_PIN ) | _BV( LCD_DC_PIN ) | _BV( LCD_CE_PIN ) | _BV( SPI_MOSI_PIN ) | _BV( SPI_CLK_PIN );

    // ��������������� ��������
    Delay();

    // ������� reset
    LCD_PORT &= ~( _BV( LCD_RST_PIN ) );
    Delay();
    LCD_PORT |= _BV ( LCD_RST_PIN );

    // ���������� SPI:
    // ��� ����������, ������� ��� ������, ����� �������, CPOL->0, CPHA->0, Clk/4
    SPCR = 0x50;

    // ��������� LCD ���������� - ������� ������� �� SCE
    LCD_PORT |= _BV( LCD_CE_PIN );

    // ���������� ������� �������
    LcdSend( 0x21, LCD_CMD ); // �������� ����������� ����� ������ (LCD Extended Commands)
    LcdSend( 0xC8, LCD_CMD ); // ��������� ������������� (LCD Vop)
    LcdSend( 0x06, LCD_CMD ); // ��������� �������������� ������������ (Temp coefficent)
    LcdSend( 0x13, LCD_CMD ); // ��������� ������� (LCD bias mode 1:48)
    LcdSend( 0x20, LCD_CMD ); // �������� ����������� ����� ������ � �������������� ��������� (LCD Standard Commands,Horizontal addressing mode)
    LcdSend( 0x0C, LCD_CMD ); // ���������� ����� (LCD in normal mode)

    // ��������� ������� �������
    LcdClear();
	memset( LcdCache, 0x20, LCD_CACHE_SIZE ); // ��� ������� - �������� � RAM (��������� ���������)
    LcdUpdate();
}



/*
 * ���                   :  LcdClear
 * ��������              :  ������� �������. ����� ���������� ��������� LcdUpdate
 * ��������(�)           :  ���
 * ������������ �������� :  ���
 */
void LcdClear ( void )
{
//    // ������� ���� �������
//    int i;
//    for ( i = 0; i < LCD_CACHE_SIZE; i++ )
//    {
//        LcdCache[i] = 0x00;
//    }

    // ����������� �� Jakub Lasinski (March 14 2009)
    memset( LcdCache, 0x00, LCD_CACHE_SIZE );
    
    // ����� ���������� ������ � ������������ ��������
    LoWaterMark = 0;
    HiWaterMark = LCD_CACHE_SIZE - 1;

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;
}



/*
 * ���                   :  LcdUpdate
 * ��������              :  �������� ��� � ��� �������
 * ��������(�)           :  ���
 * ������������ �������� :  ���
 */
void LcdUpdate (void)
{
    int i;

    if ( LoWaterMark < 0 )
        LoWaterMark = 0;
    else if ( LoWaterMark >= LCD_CACHE_SIZE )
        LoWaterMark = LCD_CACHE_SIZE - 1;

    if ( HiWaterMark < 0 )
        HiWaterMark = 0;
    else if ( HiWaterMark >= LCD_CACHE_SIZE )
        HiWaterMark = LCD_CACHE_SIZE - 1;

    #ifdef CHINA_LCD  // �������� ��� ���������� �� �� ������������� ������������

        byte x,y;

        // 102 x 64 - ������ �������������� ���������� ������ ���������� ��, ��� ���
        // ������ ������ ������������ �� ������� �� ������� ����� �� 3 �������.
        // ������� ������� �������� ���� - ������� � ������ ������ y+1, � �����
        // ������� ����� (����� ���� ���� �������, �������� � ������ ������)
                
        x = LoWaterMark % LCD_X_RES;      // ������������� ��������� ����� x
        LcdSend( 0x80 | x, LCD_CMD );     // ������������ ������ ������� LoWaterMark
        
        y = LoWaterMark / LCD_X_RES + 1;  // ������������� ��������� ����� y+1
        LcdSend( 0x40 | y, LCD_CMD );     // ������������ ������ ������� LoWaterMark

        for ( i = LoWaterMark; i <= HiWaterMark; i++ )
        {
            // �������� ������ � ����� �������
            LcdSend( LcdCache[i], LCD_DATA );
            
            x++;                 // ������ ������������ ���������� x, ����� ������� ������� �� ����� ������
            if (x >= LCD_X_RES)  // ���� ����� �� ������, �� ��������� �� ��������� ������ (x=0; y++)
            {
                // ����� ������, ����� ����� ��������� ������ ����� �������������� ������,
                // �������� ���� ��������� ��������� �����, ����� ��� �������� :)
                x=0;                
                LcdSend( 0x80, LCD_CMD );
                y++;
                LcdSend( 0x40 | y, LCD_CMD );
            }
        }

        LcdSend( 0x21, LCD_CMD );    // �������� ����������� ����� ������
        LcdSend( 0x45, LCD_CMD );    // �������� �������� �� 5 �������� ����� (������������� ������� �������, �������� � ����������)
        LcdSend( 0x20, LCD_CMD );    // �������� ����������� ����� ������ � �������������� ���������

    #else  // �������� ��� ������������� �������

        // ������������� ��������� ����� � ������������ � LoWaterMark
        LcdSend( 0x80 | ( LoWaterMark % LCD_X_RES ), LCD_CMD );
        LcdSend( 0x40 | ( LoWaterMark / LCD_X_RES ), LCD_CMD );

        // ��������� ����������� ����� ������ �������
        for ( i = LoWaterMark; i <= HiWaterMark; i++ )
        {
            // ��� ������������� ������� �� ����� ������� �� ������� � ������,
            // ����� ������ ��������������� �������� ������
            LcdSend( LcdCache[i], LCD_DATA );
        }
		
		//// ������ ���� �����
		//for ( i = 0; i <=  LCD_CACHE_SIZE - 1; i++ )
		//{
			//// ��� ������������� ������� �� ����� ������� �� ������� � ������,
			//// ����� ������ ��������������� �������� ������
			//LcdSend( LcdCache[i], LCD_DATA );
		//}

    #endif

    // ����� ���������� ������ � �������
    LoWaterMark = LCD_CACHE_SIZE - 1;
    HiWaterMark = 0;

    // ����� ����� ��������� ����
    UpdateLcd = FALSE;
}



/*
 * ���                   :  LcdSend
 * ��������              :  ���������� ������ � ���������� �������
 * ��������(�)           :  data -> ������ ��� ��������
 *                          cd   -> ������� ��� ������ (������ enum � n3310.h)
 * ������������ �������� :  ���
 */
static void LcdSend ( byte data, LcdCmdData cd )
{
    // �������� ���������� ������� (������ ������� ��������)
    LCD_PORT &= ~( _BV( LCD_CE_PIN ) );

    if ( cd == LCD_DATA )
    {
        LCD_PORT |= _BV( LCD_DC_PIN );
    }
    else
    {
        LCD_PORT &= ~( _BV( LCD_DC_PIN ) );
    }

    // �������� ������ � ���������� �������
    SPDR = data;

    // ���� ��������� ��������
    while ( (SPSR & 0x80) != 0x80 );

    // ��������� ���������� �������
    LCD_PORT |= _BV( LCD_CE_PIN );
}



/*
 * ���                   :  LcdContrast
 * ��������              :  ������������� ������������� �������
 * ��������(�)           :  �������� -> �������� �� 0x00 � 0x7F
 * ������������ �������� :  ���
 */
void LcdContrast ( byte contrast )
{
    LcdSend( 0x21, LCD_CMD );              // ����������� ����� ������
    LcdSend( 0x80 | contrast, LCD_CMD );   // ��������� ������ �������������
    LcdSend( 0x20, LCD_CMD );              // ����������� ����� ������, �������������� ���������
}



/*
 * ���                   :  Delay
 * ��������              :  ��������������� �������� ��� ��������� ������������� LCD
 * ��������(�)           :  ���
 * ������������ �������� :  ���
 */
static void Delay ( void )
{
    int i;
    for ( i = -32000; i < 32000; i++ );
	//_delay_ms(1000);
}



/*
 * ���                   :  LcdGotoXYFont
 * ��������              :  ������������� ������ � ������� x,y ������������ ������������ ������� ������
 * ��������(�)           :  x,y -> ���������� ����� ������� �������. ��������: 0,0 .. 13,5
 * ������������ �������� :  ������ ������������ �������� � n3310.h
 */
byte LcdGotoXYFont ( byte x, byte y )
{
    // �������� ������
    if( x > 13 || y > 5 ) return OUT_OF_BORDER;

    //  ���������� ���������. ��������� ��� ����� � �������� 504 ����
    LcdCacheIdx = x * 6 + y * 84;
    return OK;
}



/*
 * ���                   :  LcdChr
 * ��������              :  ������� ������ � ������� ������� �������, ����� �������������� ��������� �������
 * ��������(�)           :  size -> ������ ������. ������ enum � n3310.h
 *                          ch   -> ������ ��� ������
 * ������������ �������� :  ������ ������������ �������� � n3310lcd.h
 */
byte LcdChr ( LcdFontSize size, byte ch )
{
    byte i, c;
    byte b1, b2;
    int  tmpIdx;

    if ( LcdCacheIdx < LoWaterMark )
    {
        // ��������� ������ �������
        LoWaterMark = LcdCacheIdx;
    }

	
	if ( (ch >= 0x00) && (ch <= 0x7F) )
	{
		
	}
	else if ( (ch >= 0xC0) && (ch <= 0xFF) )
	{
		ch -= 64;
	}
	else // ��������� ������� ��� '_'
	{
		ch = 95;
	}
	

    if ( size == FONT_1X )
    {
        for ( i = 0; i < 5; i++ )
        {
            // �������� ��� ������� �� ������� � ���
            //x LcdCache[LcdCacheIdx++] = pgm_read_byte( &(FontLookup[ch][i]) ) << 1;
			//LcdCache[LcdCacheIdx++] = pgm_read_byte( &(FontLookup[ch][i]) ) << 1;
			//LcdCache[LcdCacheIdx++] = eeprom_read_byte( &(FontLookup[ch][i]) ) << 1;
			#ifdef FONT_IN_EEPROM
				LcdCache[LcdCacheIdx++] = eeprom_read_byte( &(FontLookup[ch][i]) ) << 1;
			#else
				LcdCache[LcdCacheIdx++] = pgm_read_byte( &(FontLookup[ch][i]) ) << 1;
			#endif
        }
    }
    else if ( size == FONT_2X )
    {
        tmpIdx = LcdCacheIdx - 84;
        if ( tmpIdx < LoWaterMark )
        {
            LoWaterMark = tmpIdx;
        }
        if ( tmpIdx < 0 ) return OUT_OF_BORDER;
        for ( i = 0; i < 5; i++ )
        {
            // �������� ��� ������� �� ������� � ��������� ����������
			//c = pgm_read_byte(&(FontLookup[ch][i])) << 1;
			//c = eeprom_read_byte(&(FontLookup[ch][i])) << 1;
			#ifdef FONT_IN_EEPROM
				c = eeprom_read_byte(&(FontLookup[ch][i])) << 1;
			#else
				c = pgm_read_byte(&(FontLookup[ch][i])) << 1;
			#endif
            // ����������� ��������
            // ������ �����
            b1 =  (c & 0x01) * 3;
            b1 |= (c & 0x02) * 6;
            b1 |= (c & 0x04) * 12;
            b1 |= (c & 0x08) * 24;
            c >>= 4;
            // ������ �����
            b2 =  (c & 0x01) * 3;
            b2 |= (c & 0x02) * 6;
            b2 |= (c & 0x04) * 12;
            b2 |= (c & 0x08) * 24;
            // �������� ��� ����� � ���
            LcdCache[tmpIdx++] = b1;
            LcdCache[tmpIdx++] = b1;
            LcdCache[tmpIdx + 82] = b2;
            LcdCache[tmpIdx + 83] = b2;
        }
        // ��������� x ���������� �������
        LcdCacheIdx = (LcdCacheIdx + 11) % LCD_CACHE_SIZE;
    }

    if ( LcdCacheIdx > HiWaterMark )
    {
        // ��������� ������� �������
        HiWaterMark = LcdCacheIdx;
    }
	
    // �������������� ������ ����� ���������
    LcdCache[LcdCacheIdx] = 0x00;
    // ���� �������� ������� ��������� LCD_CACHE_SIZE - 1, ��������� � ������
    if(LcdCacheIdx == (LCD_CACHE_SIZE - 1) )
    {
        LcdCacheIdx = 0;
        return OK_WITH_WRAP;
    }
	
    // ����� ������ �������������� ���������
    LcdCacheIdx++;

	
    return OK;
}



/*
 * ���                   :  LcdStr
 * ��������              :  ��� ������� ������������� ��� ������ ������ ������� �������� � RAM
 * ��������(�)           :  size      -> ������ ������. ������ enum � n3310.h
 *                          dataArray -> ������ ���������� ������ ������� ����� ����������
 * ������������ �������� :  ������ ������������ �������� � n3310lcd.h
 */
byte LcdStr ( LcdFontSize size, byte dataArray[], bool wrapLine, bool wrapDisplay  )
{
    byte tmpIdx=0;
    byte response;
    while( dataArray[ tmpIdx ] != '\0' )
    {
        // ������� ������
        response = LcdChr( size, dataArray[ tmpIdx ] );
        // �� ����� ����������� ���� ���������� OUT_OF_BORDER,
        // ������ ����� ���������� ������ �� ������ �������
        if (response == OK_WITH_WRAP && !wrapDisplay)
			return OK_WITH_WRAP;
        if(response == OUT_OF_BORDER)
			return OUT_OF_BORDER;
        // ����������� ���������
        tmpIdx++;

		// ���� ������ ����� �� ������� ������ (� ������ ����� �� ���������, ���, �������� � ������� �������)
		if (!wrapLine && ((LcdCacheIdx) % (LCD_X_RES ) == 0 ))
		{
			return OK_WITH_WRAP_LINE;
		}
    }
    return OK;
}



/*
 * ���                   :  LcdFStr
 * ��������              :  ��� ������� ������������� ��� ������ ������ ������� �������� � Flash ROM
 * ��������(�)           :  size    -> ������ ������. ������ enum � n3310.h
 *                          dataPtr -> ��������� �� ������ ������� ����� ����������
 *							wrapLine -> ���� false, �� ���� ����� �� ���� � ������, �� ������ �� �������
 *							wrapDisplay -> ���� false, �� ���� ����� �� ���� � �������, �� ������ �� ������� � ������ �������
 * ������������ �������� :  ������ ������������ �������� � n3310lcd.h
 * ������                :  LcdFStr(FONT_1X, PSTR("Hello World"));
 *                          LcdFStr(FONT_1X, &name_of_string_as_array);
 */
byte LcdFStr ( LcdFontSize size, const byte *dataPtr, bool wrapLine, bool wrapDisplay )
{
    byte c;
    byte response;
    for ( c = pgm_read_byte( dataPtr ); c; ++dataPtr, c = pgm_read_byte( dataPtr ) )
    {
        // ������� ������
        response = LcdChr( size, c );
		//if ((response == OK_WITH_WRAP_LINE || response == OK_WITH_WRAP) && !wrapLine)
			//return response; //OK_WITH_WRAP_LINE;
		if (response == OK_WITH_WRAP && !wrapDisplay)
			return OK_WITH_WRAP;
        if(response == OUT_OF_BORDER)
            return OUT_OF_BORDER;
			
		// ���� ������ ����� �� ������� ������ (� ������ ����� �� ���������, ���, �������� � ������� �������)
		if (!wrapLine && ((LcdCacheIdx) % (LCD_X_RES ) == 0 ))
		{
			return OK_WITH_WRAP_LINE;
		}
    }
    return OK;
}



/*
 * ���                   :  LcdPixel
 * ��������              :  ���������� ������� �� ���������� ����������� (x,y)
 * ��������(�)           :  x,y  -> ���������� ���������� �������
 *                          mode -> Off, On ��� Xor. ������ enum � n3310.h
 * ������������ �������� :  ������ ������������ �������� � n3310lcd.h
 */
byte LcdPixel ( byte x, byte y, LcdPixelMode mode )
{
    int  index;
    byte  offset;
    byte  data;

    // ������ �� ������ �� �������
    if ( x >= LCD_X_RES || y >= LCD_Y_RES) return OUT_OF_BORDER;

    // �������� ������� � ��������
    index = ( ( y / 8 ) * 84 ) + x;
    offset  = y - ( ( y / 8 ) * 8 );

    data = LcdCache[ index ];

    // ��������� �����

    // ����� PIXEL_OFF
    if ( mode == PIXEL_OFF )
    {
        data &= ( ~( 0x01 << offset ) );
    }
    // ����� PIXEL_ON
    else if ( mode == PIXEL_ON )
    {
        data |= ( 0x01 << offset );
    }
    // ����� PIXEL_XOR
    else if ( mode  == PIXEL_XOR )
    {
        data ^= ( 0x01 << offset );
    }

    // ������������� ��������� �������� � ���
    LcdCache[ index ] = data;

    if ( index < LoWaterMark )
    {
        // ��������� ������ �������
        LoWaterMark = index;
    }

    if ( index > HiWaterMark )
    {
        // ��������� ������� �������
        HiWaterMark = index;
    }
    return OK;
}



/*
 * ���                   :  LcdLine
 * ��������              :  ������ ����� ����� ����� ������� �� ������� (�������� ����������)
 * ��������(�)           :  x1, y1  -> ���������� ���������� ������ �����
 *                          x2, y2  -> ���������� ���������� ����� �����
 *                          mode    -> Off, On ��� Xor. ������ enum � n3310.h
 * ������������ �������� :  ������ ������������ �������� � n3310lcd.h
 */
byte LcdLine ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode )
{
    int dx, dy, stepx, stepy, fraction;
    byte response;

    // dy   y2 - y1
    // -- = -------
    // dx   x2 - x1

    dy = y2 - y1;
    dx = x2 - x1;

    // dy �������������
    if ( dy < 0 )
    {
        dy    = -dy;
        stepy = -1;
    }
    else
    {
        stepy = 1;
    }

    // dx �������������
    if ( dx < 0 )
    {
        dx    = -dx;
        stepx = -1;
    }
    else
    {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    // ������ ��������� �����
    response = LcdPixel( x1, y1, mode );
    if(response)
        return response;

    // ������ ��������� ����� �� �����
    if ( dx > dy )
    {
        fraction = dy - ( dx >> 1);
        while ( x1 != x2 )
        {
            if ( fraction >= 0 )
            {
                y1 += stepy;
                fraction -= dx;
            }
            x1 += stepx;
            fraction += dy;

            response = LcdPixel( x1, y1, mode );
            if(response)
                return response;

        }
    }
    else
    {
        fraction = dx - ( dy >> 1);
        while ( y1 != y2 )
        {
            if ( fraction >= 0 )
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;

            response = LcdPixel( x1, y1, mode );
            if(response)
                return response;
        }
    }

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;
    return OK;
}



/*
 * ���                   :  LcdCircle
 * ��������              :  ������ ���������� (�������� ����������)
 * ��������(�)           :  x, y   -> ���������� ���������� ������
 *                          radius -> ������ ����������
 *                          mode   -> Off, On ��� Xor. ������ enum � n3310.h
 * ������������ �������� :  ������ ������������ �������� � n3310lcd.h
 */
byte LcdCircle(byte x, byte y, byte radius, LcdPixelMode mode)
{
    signed char xc = 0;
    signed char yc = 0;
    signed char p = 0;

    if ( x >= LCD_X_RES || y >= LCD_Y_RES) return OUT_OF_BORDER;

    yc = radius;
    p = 3 - (radius<<1);
    while (xc <= yc)  
    {
        LcdPixel(x + xc, y + yc, mode);
        LcdPixel(x + xc, y - yc, mode);
        LcdPixel(x - xc, y + yc, mode);
        LcdPixel(x - xc, y - yc, mode);
        LcdPixel(x + yc, y + xc, mode);
        LcdPixel(x + yc, y - xc, mode);
        LcdPixel(x - yc, y + xc, mode);
        LcdPixel(x - yc, y - xc, mode);
        if (p < 0) p += (xc++ << 2) + 6;
            else p += ((xc++ - yc--)<<2) + 10;
    }

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;
    return OK;
}




/*
 * ���                   :  LcdRect
 * ��������              :  ������ ������������� �������������
 * ��������(�)           :  x1    -> ���������� ���������� x ������ �������� ����
 *                          y1    -> ���������� ���������� y ������ �������� ����
 *                          x2    -> ���������� ���������� x ������� ������� ����
 *                          y2    -> ���������� ���������� y ������� ������� ����
 *                          mode  -> Off, On ��� Xor. ������ enum � n3310.h
 * ������������ �������� :  ������ ������������ �������� � n3310lcd.h
 */
byte LcdRect ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode )
{
    byte tmpIdx;

    // �������� ������
    if ( ( x1 >= LCD_X_RES) ||  ( x2 >= LCD_X_RES) || ( y1 >= LCD_Y_RES) || ( y2 >= LCD_Y_RES) )
        return OUT_OF_BORDER;

    if ( ( x2 > x1 ) && ( y2 > y1 ) )
    {
        // ������ �������������� �����
        for ( tmpIdx = x1; tmpIdx <= x2; tmpIdx++ )
        {
            LcdPixel( tmpIdx, y1, mode );
            LcdPixel( tmpIdx, y2, mode );
        }

        // ������ ������������ �����
        for ( tmpIdx = y1; tmpIdx <= y2; tmpIdx++ )
        {
            LcdPixel( x1, tmpIdx, mode );
            LcdPixel( x2, tmpIdx, mode );
        }

        // ��������� ����� ��������� ����
        UpdateLcd = TRUE;
    }
	
    return OK;
}



/*
 * ���                   :  LcdImage
 * ��������              :  ������ �������� �� ������� ������������ � Flash ROM
 * ��������(�)           :  ��������� �� ������ ��������
 * ������������ �������� :  ���
 */
void LcdImage ( const byte *imageData )
{
//    // ������������� ��������� ����
//    LcdCacheIdx = 0;
//    // � �������� ����
//    for ( LcdCacheIdx = 0; LcdCacheIdx < LCD_CACHE_SIZE; LcdCacheIdx++ )
//    {
//        // �������� ������ �� ������� � ���
//        LcdCache[LcdCacheIdx] = pgm_read_byte( imageData++ );
//    }
    
    // ����������� �� Jakub Lasinski (March 14 2009)
    memcpy_P( LcdCache, imageData, LCD_CACHE_SIZE );  // ���� ����� ��� � ����, �� �������� ������ ������ � ������� �����������
    
    // ����� ���������� ������ � ������������ ��������
    LoWaterMark = 0;
    HiWaterMark = LCD_CACHE_SIZE - 1;

    // ��������� ����� ��������� ����
    UpdateLcd = TRUE;
}





void LcdDrawFastVLine(int8_t x, int8_t y, int8_t h) {
	for(int8_t i=0; i<h; i++){
		LcdPixel(x,y+i, PIXEL_ON);
	}
}

void LcdDrawFastHLine(int8_t x, int8_t y, int8_t w) {
	for(int8_t i=0; i<w; i++){
		LcdPixel(x+i,y, PIXEL_ON);
	}
}



/*
	������� ������� � bitmap {Width, Height, {BYTES}}

*/
bool LcdGetBitmapPixel(const uint8_t* bitmap, uint8_t x, uint8_t y)
{
	return pgm_read_byte(bitmap+2 + y * ((pgm_read_byte(bitmap)+7)/8) + (x >> 3)) & (0b10000000 >> (x % 8));
}
unsigned int LcdGetBitmapWidth(const uint8_t* bitmap)
{
	return pgm_read_byte(bitmap);
}
unsigned int LcdGetBitmapHeight(const uint8_t* bitmap)
{
	return pgm_read_byte(bitmap+1);
}

/*
	����� �������� �� ������� Gamebuino (https://github.com/Rodot/Gamebuino/blob/64de814fc8cd746fe66a7074b2c18960067aad1d/utility/Display.cpp#L495)
	{Width, Height, {BYTES}}
	BYTES - ����� ��������, �� LCD Assistant �������� ���������� ��������� Byte Orientation - Horizontal, 8 pixels/byte
		������:
			const byte road[] PROGMEM = {  16,16 ,0x0,0x0,0x20,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x10,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x0,0x0,0x40,0x0,0x0,0x0,0x0,0x8,0x4,0x0,0x0,0x0,};
			..
			LcdDrawBitmap(0, 0, road);
*/
void LcdDrawBitmap(int8_t x, int8_t y, const uint8_t *bitmap) 
{
   int8_t w = pgm_read_byte(bitmap);
   int8_t h = pgm_read_byte(bitmap + 1);
   bitmap = bitmap + 2; //add an offset to the pointer to start after the width and height
#if (ENABLE_BITMAPS > 0)
/*   original code
    int8_t i, j, byteWidth = (w + 7) / 8;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (B10000000 >> (i % 8))) {
                drawPixel(x + i, y + j);
            }
        }
    }
  */
  uint8_t * buffer = LcdCache; //getBuffer();
  const uint8_t col = 1; //color; // 0-WHITE; 1-BLACK; 2-GRAY
  const uint8_t bw = (w+7) / 8;
  
  // clip
  if (x >= LCD_X_RES)
    return;
  if (x + w <= 0)
    return;
  if (y >= LCD_Y_RES)
    return;
  if (y + h <= 0)
    return;
  if (y < 0)
    h += y, bitmap -= bw * y, y = 0;
  if (y + h > LCD_Y_RES)
    h = LCD_Y_RES - y;  
  uint8_t x1 = max(0, x);
  uint8_t x2 = min(LCD_X_RES, x + w);
  
//#ifdef ENABLE_GRAYSCALE
   //uint8_t g = y ^ frameCount;
//#endif  

  // draw
  uint8_t first_bitmap_mask = 0x80 >> ((x1 - x) & 7);
  const uint8_t * bitmap_line = bitmap + (x1 - x) / 8;
  uint8_t screen_mask = 0x01 << (y % 8);
  uint8_t * screen_row = buffer + (y / 8) * LCD_X_RES + x1;  
  for (uint8_t dy=0; dy<h; dy++, bitmap_line+=bw)
  {
    const uint8_t * bitmap_ptr = bitmap_line;    
    uint8_t bitmap_mask = first_bitmap_mask;    
    uint8_t pixels = pgm_read_byte(bitmap_ptr);
    uint8_t * dst = screen_row;
    
    if (col == 1)
      for (uint8_t sx=x1; sx<x2; sx++, dst++)
      {
        if (pixels & bitmap_mask)
          *dst |= screen_mask;
        bitmap_mask >>= 1;
        if (!bitmap_mask)
        {
          bitmap_mask = 0x80;
          pixels = pgm_read_byte(++bitmap_ptr);
        }
      }
    else if (col == 0)
    {
      uint8_t inv_screen_mask = ~screen_mask;
      for (uint8_t sx=x1; sx<x2; sx++, dst++)
      {
        if (pixels & bitmap_mask)
          *dst &= inv_screen_mask;
        bitmap_mask >>= 1;
        if (!bitmap_mask)
        {
          bitmap_mask = 0x80;
          pixels = pgm_read_byte(++bitmap_ptr);
        }
      }
    }
//#ifdef ENABLE_GRAYSCALE
    //else if (col == 2)
    //{
      //uint8_t inv_screen_mask = ~screen_mask;
      //for (uint8_t sx=x1; sx<x2; sx++, dst++)
      //{
        //if (pixels & bitmap_mask)
        //{
         //if ((sx^g) & 1)
            //*dst |= screen_mask;
          //else
           //*dst &= inv_screen_mask;
        //}
        //bitmap_mask >>= 1;
        //if (!bitmap_mask)
        //{
          //bitmap_mask = 0x80;
          //pixels = pgm_read_byte(++bitmap_ptr);
        //}
      //}
       //g ^= 1;
    //}
//#endif
   else // invert
      for (uint8_t sx=x1; sx<x2; sx++, dst++)
      {
        if (pixels & bitmap_mask)
          *dst ^= screen_mask;
        bitmap_mask >>= 1;
        if (!bitmap_mask)
        {
          bitmap_mask = 0x80;
          pixels = pgm_read_byte(++bitmap_ptr);
        }
      }
    
    screen_mask <<= 1;
    if (!screen_mask)
    {
      screen_mask = 1;
      screen_row += LCD_X_RES;
    }
  }
#else
   LcdRect(x, y, x+w, y+h, PIXEL_ON);
#endif
}





void LcdDrawBitmap(int8_t x, int8_t y, const uint8_t *bitmap, LcdRotator rotation, LcdFlip flip) 
{
	if((rotation == NOROT) && (flip == NOFLIP)){
		LcdDrawBitmap(x,y,bitmap); //use the faster algorithm
		return;
	}
	uint8_t w = pgm_read_byte(bitmap);
	uint8_t h = pgm_read_byte(bitmap + 1);
	bitmap = bitmap + 2; //add an offset to the pointer to start after the width and height
	#if (ENABLE_BITMAPS > 0)
	int8_t i, j, //coordinates in the raw bitmap
	k, l, //coordinates in the rotated/flipped bitmap
	byteNum, bitNum, byteWidth = (w + 7) >> 3;

	//rotation %= 4;
	rotation = (LcdRotator)(rotation % 4);

	for (i = 0; i < w; i++) {
		byteNum = i / 8;
		bitNum = i % 8;
		for (j = 0; j < h; j++) {
			if (pgm_read_byte(bitmap + j * byteWidth + byteNum) & (0b10000000 >> bitNum)) {
				switch (rotation) {
					case NOROT: //no rotation
					k = i;
					l = j;
					break;
					case ROTCCW: //90� counter-clockwise
					k = j;
					l = w - i - 1;
					break;
					case ROT180: //180�
					k = w - i - 1;
					l = h - j - 1;
					break;
					case ROTCW: //90� clockwise
					k = h - j - 1;
					l = i;
					break;
				}
				if (flip) {
					//flip %= 4;
					flip = (LcdFlip)(flip % 4);
					if (flip & 0b00000001) { //horizontal flip
						k = w - k - 1;
					}
					if (flip & 0b00000010) { //vertical flip
						l = h - l;
					}
				}
				k += x; //place the bitmap on the screen
				l += y;
				//drawPixel(k, l);
				LcdPixel(k, l, PIXEL_ON);
			}
		}
	}
	#else
	LcdRect(x, y, x+w, y+h, PIXEL_ON);
	#endif
}













byte LcdPrint(int n, int base)
{
	return LcdPrint((long) n, base);
}
byte LcdPrint(unsigned int n, int base)
{
	return LcdPrint((unsigned long) n, base);
}



byte LcdPrint(long n, int base)
{
	if (base == 0) {
		return 0;//write(n);
		} else if (base == 10) {
		if (n < 0) {
			//int t = print('-');
			int t = 1; LcdStr(FONT_1X, (unsigned char*)"-");

			n = -n;
			return __printNumber(n, 10) + t;
		}
		return __printNumber(n, 10);
		} else {
		return __printNumber(n, base);
	}
}
byte LcdPrint(unsigned long n, int base)
{
	if (base == 0) return 0;//write(n);
	else return __printNumber(n, base);
}


byte __printNumber(unsigned long n, byte base)
{
	char buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
	char *str = &buf[sizeof(buf) - 1];

	*str = '\0';

	// prevent crash if called with base == 1
	if (base < 2) base = 10;

	do {
		char c = n % base;
		n /= base;

		*--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while(n);


	LcdStr(FONT_1X, (byte*)str);
	return 1; //write(str); // TODO: ������ ���������� ���������� ������������ ��������
}