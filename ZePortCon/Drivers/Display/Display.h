/*
 * Display.h
 *
 * Created: 21.09.2017 20:13:39
 *  Author: Kirill
 */ 


/*
	обертка на драйвер LCD
	используется всей системой (в частности zeos)
*/




#ifndef DISPLAY_H_
#define DISPLAY_H_


#include "../Lcd/n3310.h"



class Display
{

public:

	// sys
	
	void Init       ( void ) { LcdInit(); }
	void Clear      ( void ) { LcdClear(); }
	void Update     ( void ) { LcdUpdate(); }
	void Image      ( const byte *imageData ) { LcdImage(imageData); }
	void Contrast   ( byte contrast ) { LcdContrast(contrast); }
	byte GotoXYFont ( byte x, byte y ) { return LcdGotoXYFont(x, y); }
	byte Goto	   ( byte x, byte y ) { return LcdGotoXYFont(x, y); }
	byte Chr        ( LcdFontSize size, byte ch ) { return LcdChr(size, ch); }
	byte Str        ( LcdFontSize size, byte dataArray[], bool wrapLine = false, bool wrapDisplay = false ) { return LcdStr(size, dataArray, wrapLine, wrapDisplay); }
	byte FStr       ( LcdFontSize size, const byte *dataPtr, bool wrapLine = false, bool wrapDisplay = false ) { return LcdFStr(size, dataPtr, wrapLine, wrapDisplay); }
	byte Str        ( byte dataArray[], bool wrapLine = false, bool wrapDisplay = false ) { return LcdStr(FONT_1X, dataArray, wrapLine, wrapDisplay); }
	byte FStr	   ( const byte *dataPtr, bool wrapLine = false, bool wrapDisplay = false ) { return LcdFStr(FONT_1X, dataPtr, wrapLine, wrapDisplay); }
			
	byte Pixel      ( byte x, byte y, LcdPixelMode mode ) { return LcdPixel(x, y, mode); }
	byte Pixel      ( byte x, byte y ) { return LcdPixel(x, y, PIXEL_ON); }
	byte Line       ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode ) { return LcdLine(x1, y1, x2, y2, mode); }
	byte Line       ( byte x1, byte y1, byte x2, byte y2 ) { return LcdLine(x1, y1, x2, y2, PIXEL_ON); }
	byte Circle     ( byte x, byte y, byte radius, LcdPixelMode mode) { return LcdCircle(x, y, radius, mode); }
	byte Rect       ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode ) { return LcdRect(x1, y1, x2, y2, mode); }
	int  GetCacheIdx() { return LcdGetCacheIdx(); }

	byte Print(int n, int base = DEC) { return LcdPrint(n, base); }
	byte Print(unsigned int n, int base = DEC) { return LcdPrint(n, base); }
	byte Print(long n, int base = DEC) { return LcdPrint(n, base); }
	byte Print(unsigned long n, int base = DEC) { return LcdPrint(n, base); }

	
	//

	void DrawFastVLine(int8_t x, int8_t y, int8_t h) { LcdDrawFastVLine(x, y, h); }
	void DrawFastHLine(int8_t x, int8_t y, int8_t w) { LcdDrawFastHLine(x, y, w); }


	// bitmap 

	// формат как в Gamebuino {Width, Height, {BYTES}}
	// BYTES - байты пикселей (из LCD Assistant подобных приложений указывать Byte Orientation - Horizontal, 8 pixels/byte)
	/*
		пример:
			const byte road[] PROGMEM = {  16,16 ,0x0,0x0,0x20,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x10,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,0x0,0x0,0x40,0x0,0x0,0x0,0x0,0x8,0x4,0x0,0x0,0x0,};
			..
			zeos->display.DrawBitmap(0, 0, road);
	*/

	void DrawBitmap(int8_t x, int8_t y, const uint8_t *bitmap) { LcdDrawBitmap(x,y, bitmap); }
	void DrawBitmap(int8_t x, int8_t y, const uint8_t *bitmap, LcdRotator rotation, LcdFlip flip) { LcdDrawBitmap(x,y, bitmap, rotation, flip); }
	bool GetBitmapPixel(const uint8_t* bitmap, uint8_t x, uint8_t y) { return LcdGetBitmapPixel(bitmap, x, y); }
	unsigned int GetBitmapWidth(const uint8_t* bitmap) { return LcdGetBitmapWidth(bitmap); }
	unsigned int GetBitmapHeight(const uint8_t* bitmap) { return LcdGetBitmapHeight(bitmap); }
	void DrawBitmapOnCenter(const uint8_t* bitmap) { DrawBitmap((LCD_WIDTH-GetBitmapWidth(bitmap))/2, (LCD_HEIGHT-GetBitmapHeight(bitmap))/2, bitmap); }


	// helpers

	void DrawCenterText(const byte *dataPtr, byte xOffset = 0)
	{
		Clear();
		Goto(xOffset, 2);
		FStr(dataPtr);
		Line(2, 14, 85, 14);	// hor up
		Line(2, 26, 85, 26);	// hor bottom
		Line(22, 10, 65, 10);	// hor up
		Line(22, 30, 65, 30);	// hor bottom
		Update();
	}

	void DrawErrorText(const byte *dataPtr, byte xOffset = 0)
	{
		Clear();
		Goto(xOffset, 2);
		FStr(dataPtr);
		Line(22, 10, 65, 10);	// hor up
		Line(22, 30, 65, 30);	// hor bottom
		Update();
	}

private:	
	

};


#endif /* DISPLAY_H_ */