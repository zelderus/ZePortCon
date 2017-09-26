/*
 * Имя          :  n3310.h
 *
 * Описание     :  Это заголовочный файл для драйвера графического LCD от Nokia 3310, а также его китайских клонов.
 *                 Базируется на коде библиотек написанных Sylvain Bissonnette и Fandi Gunawan:
 *                 http://www.microsyl.com/index.php/2010/03/24/nokia-lcd-library/
 *                 http://fandigunawan.wordpress.com/2008/06/18/lcd-nokia-3310-pcd8544-driver-in-winavravr-gcc/
 *                 Основные отличия между оригиналом и клоном хорошо описаны в статье от Aheir:
 *                 http://radiokot.ru/articles/29/
 *
 * Автор        :  Xander Gresolio <xugres@gmail.com>
 * Веб-страница :  http://we.easyelectronics.ru/profile/XANDER/
 *
 * Лицензия     :  GPL v3.0
 *
 * Компилятор   :  WinAVR, GCC for AVR platform
 */

#ifndef _N3310_H_
#define _N3310_H_



#include "../../settings.h"






#define ENABLE_BITMAPS		1


// Разрешение дисплея в пикселях
#define LCD_X_RES                  LCD_WIDTH     // разрешение по горизонтали
#define LCD_Y_RES                  LCD_HEIGHT    // разрешение по вертикали

//// Настройки для рисования группы прямоугольников функцией LcdBars ( byte data[], byte numbBars, byte width, byte multiplier )
//#define EMPTY_SPACE_BARS           2     // расстояние между прямоугольниками
//#define BAR_X                      30    // координата x
//#define BAR_Y                      47    // координата y

// Размер кэша ( 84 * 48 ) / 8 = 504 байта
#define LCD_CACHE_SIZE             ( ( LCD_X_RES * LCD_Y_RES ) / 8 )

#define FALSE                      0
#define TRUE                       1

// Для возвращаемых значений
#define OK                         0   // Безошибочная отрисовка
#define OUT_OF_BORDER              1   // Выход за границы дисплея
#define OK_WITH_WRAP               2   // Переход на начало (ситуация автоинкремента указателя курсора при выводе длинного текста)
#define OK_WITH_WRAP_LINE          3   // Переход на новую строкуо (ситуация автоинкремента указателя курсора при выводе длинного текста)

typedef unsigned char              byte;


#define DEC							10
#define HEX							16
#define OCT							8
#define BIN							2



// Перечисления
typedef enum
{
    LCD_CMD  = 0,     // Команда
    LCD_DATA = 1      // Данные

} LcdCmdData;

typedef enum
{
    PIXEL_OFF =  0,   // Погасить пиксели дисплея
    PIXEL_ON  =  1,   // Включить пиксели дисплея
    PIXEL_XOR =  2    // Инвертировать пиксели

} LcdPixelMode;

typedef enum
{
    FONT_1X = 1,      // Обычный размер шрифта 5x7
    FONT_2X = 2       // Увеличенный размер шрифта

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




// Прототипы функций, детальную информацию смотрим внутри n3310lcd.c
void LcdInit       ( void );   // Инициализация
void LcdClear      ( void );   // Очистка буфера
void LcdUpdate     ( void );   // Копирование буфера в ОЗУ дисплея
void LcdImage      ( const byte *imageData );   // Рисование картинки из массива в Flash ROM
void LcdContrast   ( byte contrast );   // Установка контрастности дисплея
byte LcdGotoXYFont ( byte x, byte y );   // Установка курсора в позицию x,y
byte LcdChr        ( LcdFontSize size, byte ch );   // Вывод символа в текущей позиции
byte LcdStr        ( LcdFontSize size, byte dataArray[], bool wrapLine = false, bool wrapDisplay = false );   // Вывод строки сохраненной в RAM
byte LcdFStr       ( LcdFontSize size, const byte *dataPtr, bool wrapLine = false, bool wrapDisplay = false );   // Вывод строки сохраненной в Flash ROM
byte LcdPixel      ( byte x, byte y, LcdPixelMode mode );   // Точка
byte LcdLine       ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode );   // Линия
byte LcdCircle     ( byte x, byte y, byte radius, LcdPixelMode mode);   // Окружность
byte LcdRect       ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode );   // Прямоугольник
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
