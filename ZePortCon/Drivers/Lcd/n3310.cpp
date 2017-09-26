/*
 * Имя          :  n3310.c
 *
 * Описание     :  Это драйвер для графического LCD от Nokia 3310, а также его китайских клонов.
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
 *
 * История      :
 * Версия 1.0 (06.08.2011)
 * + Первая версия
 * + Добавлена поддержка китайских клонов LCD Nokia 3310
 * + Полный перевод комментариев к исходному коду драйвера
 * + Таблица символов драйвера дополнена кириллицей (упрощенная Windows-1251)
 * + Добавлена функция рисования окружностей LcdCircle
 * - Исправлены ошибки в проверке корректности координат при вызове функций рисования
 * - Исправлена ошибка в функции LcdSingleBar (неверная отрисовка по y)
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





// Прототипы приватных функций драйвера

static void LcdSend    ( byte data, LcdCmdData cd );
static void Delay      ( void );

// Глобальные переменные

// Кэш в ОЗУ 84*48 бит или 504 байта
static byte  LcdCache [ LCD_CACHE_SIZE ];

// Чтобы не обновлять весь дисплей, а лишь ту часть что изменилась,
// будем отмечать две границы кэша где произошли изменения. Затем
// можно копировать эту часть кэша между границами в ОЗУ дисплея.
static int   LoWaterMark;   // нижняя граница
static int   HiWaterMark;   // верхняя граница

// Указатель для работы с LcdCache[]
static int   LcdCacheIdx;

// Флаг изменений кэша
static byte  UpdateLcd;


int LcdGetCacheIdx()
{
	return LcdCacheIdx;
}


/*
 * Имя                   :  LcdInit
 * Описание              :  Производит инициализацию порта и SPI МК, контроллера LCD
 * Аргумент(ы)           :  Нет
 * Возвращаемое значение :  Нет
 */
void LcdInit ( void )
{

    // Pull-up на вывод подключенный к reset дисплея
    LCD_PORT |= _BV ( LCD_RST_PIN );

    // Устанавливаем нужные биты порта на выход
    LCD_DDR |= _BV( LCD_RST_PIN ) | _BV( LCD_DC_PIN ) | _BV( LCD_CE_PIN ) | _BV( SPI_MOSI_PIN ) | _BV( SPI_CLK_PIN );

    // Некалиброванная задержка
    Delay();

    // Дергаем reset
    LCD_PORT &= ~( _BV( LCD_RST_PIN ) );
    Delay();
    LCD_PORT |= _BV ( LCD_RST_PIN );

    // Активируем SPI:
    // без прерываний, старший бит первый, режим мастера, CPOL->0, CPHA->0, Clk/4
    SPCR = 0x50;

    // Отключаем LCD контроллер - высокий уровень на SCE
    LCD_PORT |= _BV( LCD_CE_PIN );

    // Отправляем команды дисплею
    LcdSend( 0x21, LCD_CMD ); // Включаем расширенный набор команд (LCD Extended Commands)
    LcdSend( 0xC8, LCD_CMD ); // Установка контрастности (LCD Vop)
    LcdSend( 0x06, LCD_CMD ); // Установка температурного коэффициента (Temp coefficent)
    LcdSend( 0x13, LCD_CMD ); // Настройка питания (LCD bias mode 1:48)
    LcdSend( 0x20, LCD_CMD ); // Включаем стандартный набор команд и горизонтальную адресацию (LCD Standard Commands,Horizontal addressing mode)
    LcdSend( 0x0C, LCD_CMD ); // Нормальный режим (LCD in normal mode)

    // Первичная очистка дисплея
    LcdClear();
	memset( LcdCache, 0x20, LCD_CACHE_SIZE ); // для отладки - просмотр в RAM (заполнено пробелами)
    LcdUpdate();
}



/*
 * Имя                   :  LcdClear
 * Описание              :  Очищает дисплей. Далее необходимо выполнить LcdUpdate
 * Аргумент(ы)           :  Нет
 * Возвращаемое значение :  Нет
 */
void LcdClear ( void )
{
//    // Очистка кэша дисплея
//    int i;
//    for ( i = 0; i < LCD_CACHE_SIZE; i++ )
//    {
//        LcdCache[i] = 0x00;
//    }

    // Оптимизация от Jakub Lasinski (March 14 2009)
    memset( LcdCache, 0x00, LCD_CACHE_SIZE );
    
    // Сброс указателей границ в максимальное значение
    LoWaterMark = 0;
    HiWaterMark = LCD_CACHE_SIZE - 1;

    // Установка флага изменений кэша
    UpdateLcd = TRUE;
}



/*
 * Имя                   :  LcdUpdate
 * Описание              :  Копирует кэш в ОЗУ дисплея
 * Аргумент(ы)           :  Нет
 * Возвращаемое значение :  Нет
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

    #ifdef CHINA_LCD  // Алгоритм для китайского ЖК из нестандартным контроллером

        byte x,y;

        // 102 x 64 - таково предполагаемое разрешение буфера китайского ЖК, при чем
        // память буфера отображается на дисплей со сдвигом вверх на 3 пикселя.
        // Поэтому выводим картинку ниже - начиная с второй строки y+1, а потом
        // сдвинем вверх (опять таки фича китайца, полезная в данном случае)
                
        x = LoWaterMark % LCD_X_RES;      // Устанавливаем начальный адрес x
        LcdSend( 0x80 | x, LCD_CMD );     // относительно нижней границы LoWaterMark
        
        y = LoWaterMark / LCD_X_RES + 1;  // Устанавливаем начальный адрес y+1
        LcdSend( 0x40 | y, LCD_CMD );     // относительно нижней границы LoWaterMark

        for ( i = LoWaterMark; i <= HiWaterMark; i++ )
        {
            // передаем данные в буфер дисплея
            LcdSend( LcdCache[i], LCD_DATA );
            
            x++;                 // заодно подсчитываем координату x, чтобы вовремя перейти на новую строку
            if (x >= LCD_X_RES)  // если вышли за предел, то переходим на следующую строку (x=0; y++)
            {
                // проще говоря, чтобы верно заполнить нужную часть нестандартного буфера,
                // придется явно указывать требуемый адрес, иначе все поплывет :)
                x=0;                
                LcdSend( 0x80, LCD_CMD );
                y++;
                LcdSend( 0x40 | y, LCD_CMD );
            }
        }

        LcdSend( 0x21, LCD_CMD );    // Включаем расширенный набор команд
        LcdSend( 0x45, LCD_CMD );    // Сдвигаем картинку на 5 пикселей вверх (нестандартная команда китайца, оригинал её игнорирует)
        LcdSend( 0x20, LCD_CMD );    // Включаем стандартный набор команд и горизонтальную адресацию

    #else  // Алгоритм для оригинального дисплея

        // Устанавливаем начальный адрес в соответствии к LoWaterMark
        LcdSend( 0x80 | ( LoWaterMark % LCD_X_RES ), LCD_CMD );
        LcdSend( 0x40 | ( LoWaterMark / LCD_X_RES ), LCD_CMD );

        // Обновляем необходимую часть буфера дисплея
        for ( i = LoWaterMark; i <= HiWaterMark; i++ )
        {
            // Для оригинального дисплея не нужно следить за адресом в буфере,
            // можно просто последовательно выводить данные
            LcdSend( LcdCache[i], LCD_DATA );
        }
		
		//// рисуем весь буфер
		//for ( i = 0; i <=  LCD_CACHE_SIZE - 1; i++ )
		//{
			//// Для оригинального дисплея не нужно следить за адресом в буфере,
			//// можно просто последовательно выводить данные
			//LcdSend( LcdCache[i], LCD_DATA );
		//}

    #endif

    // Сброс указателей границ в пустоту
    LoWaterMark = LCD_CACHE_SIZE - 1;
    HiWaterMark = 0;

    // Сброс флага изменений кэша
    UpdateLcd = FALSE;
}



/*
 * Имя                   :  LcdSend
 * Описание              :  Отправляет данные в контроллер дисплея
 * Аргумент(ы)           :  data -> данные для отправки
 *                          cd   -> команда или данные (смотри enum в n3310.h)
 * Возвращаемое значение :  Нет
 */
static void LcdSend ( byte data, LcdCmdData cd )
{
    // Включаем контроллер дисплея (низкий уровень активный)
    LCD_PORT &= ~( _BV( LCD_CE_PIN ) );

    if ( cd == LCD_DATA )
    {
        LCD_PORT |= _BV( LCD_DC_PIN );
    }
    else
    {
        LCD_PORT &= ~( _BV( LCD_DC_PIN ) );
    }

    // Отправка данных в контроллер дисплея
    SPDR = data;

    // Ждем окончания передачи
    while ( (SPSR & 0x80) != 0x80 );

    // Отключаем контроллер дисплея
    LCD_PORT |= _BV( LCD_CE_PIN );
}



/*
 * Имя                   :  LcdContrast
 * Описание              :  Устанавливает контрастность дисплея
 * Аргумент(ы)           :  контраст -> значение от 0x00 к 0x7F
 * Возвращаемое значение :  Нет
 */
void LcdContrast ( byte contrast )
{
    LcdSend( 0x21, LCD_CMD );              // Расширенный набор команд
    LcdSend( 0x80 | contrast, LCD_CMD );   // Установка уровня контрастности
    LcdSend( 0x20, LCD_CMD );              // Стандартный набор команд, горизонтальная адресация
}



/*
 * Имя                   :  Delay
 * Описание              :  Некалиброванная задержка для процедуры инициализации LCD
 * Аргумент(ы)           :  Нет
 * Возвращаемое значение :  Нет
 */
static void Delay ( void )
{
    int i;
    for ( i = -32000; i < 32000; i++ );
	//_delay_ms(1000);
}



/*
 * Имя                   :  LcdGotoXYFont
 * Описание              :  Устанавливает курсор в позицию x,y относительно стандартного размера шрифта
 * Аргумент(ы)           :  x,y -> координаты новой позиции курсора. Значения: 0,0 .. 13,5
 * Возвращаемое значение :  смотри возвращаемое значение в n3310.h
 */
byte LcdGotoXYFont ( byte x, byte y )
{
    // Проверка границ
    if( x > 13 || y > 5 ) return OUT_OF_BORDER;

    //  Вычисление указателя. Определен как адрес в пределах 504 байт
    LcdCacheIdx = x * 6 + y * 84;
    return OK;
}



/*
 * Имя                   :  LcdChr
 * Описание              :  Выводит символ в текущей позиции курсора, затем инкрементирует положение курсора
 * Аргумент(ы)           :  size -> размер шрифта. Смотри enum в n3310.h
 *                          ch   -> символ для вывода
 * Возвращаемое значение :  смотри возвращаемое значение в n3310lcd.h
 */
byte LcdChr ( LcdFontSize size, byte ch )
{
    byte i, c;
    byte b1, b2;
    int  tmpIdx;

    if ( LcdCacheIdx < LoWaterMark )
    {
        // Обновляем нижнюю границу
        LoWaterMark = LcdCacheIdx;
    }

	
	if ( (ch >= 0x00) && (ch <= 0x7F) )
	{
		
	}
	else if ( (ch >= 0xC0) && (ch <= 0xFF) )
	{
		ch -= 64;
	}
	else // Остальные выводим как '_'
	{
		ch = 95;
	}
	

    if ( size == FONT_1X )
    {
        for ( i = 0; i < 5; i++ )
        {
            // Копируем вид символа из таблицы в кэш
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
            // Копируем вид символа из таблицы у временную переменную
			//c = pgm_read_byte(&(FontLookup[ch][i])) << 1;
			//c = eeprom_read_byte(&(FontLookup[ch][i])) << 1;
			#ifdef FONT_IN_EEPROM
				c = eeprom_read_byte(&(FontLookup[ch][i])) << 1;
			#else
				c = pgm_read_byte(&(FontLookup[ch][i])) << 1;
			#endif
            // Увеличиваем картинку
            // Первую часть
            b1 =  (c & 0x01) * 3;
            b1 |= (c & 0x02) * 6;
            b1 |= (c & 0x04) * 12;
            b1 |= (c & 0x08) * 24;
            c >>= 4;
            // Вторую часть
            b2 =  (c & 0x01) * 3;
            b2 |= (c & 0x02) * 6;
            b2 |= (c & 0x04) * 12;
            b2 |= (c & 0x08) * 24;
            // Копируем две части в кэш
            LcdCache[tmpIdx++] = b1;
            LcdCache[tmpIdx++] = b1;
            LcdCache[tmpIdx + 82] = b2;
            LcdCache[tmpIdx + 83] = b2;
        }
        // Обновляем x координату курсора
        LcdCacheIdx = (LcdCacheIdx + 11) % LCD_CACHE_SIZE;
    }

    if ( LcdCacheIdx > HiWaterMark )
    {
        // Обновляем верхнюю границу
        HiWaterMark = LcdCacheIdx;
    }
	
    // Горизонтальный разрыв между символами
    LcdCache[LcdCacheIdx] = 0x00;
    // Если достигли позицию указателя LCD_CACHE_SIZE - 1, переходим в начало
    if(LcdCacheIdx == (LCD_CACHE_SIZE - 1) )
    {
        LcdCacheIdx = 0;
        return OK_WITH_WRAP;
    }
	
    // Иначе просто инкрементируем указатель
    LcdCacheIdx++;

	
    return OK;
}



/*
 * Имя                   :  LcdStr
 * Описание              :  Эта функция предназначена для печати строки которая хранится в RAM
 * Аргумент(ы)           :  size      -> размер шрифта. Смотри enum в n3310.h
 *                          dataArray -> массив содержащий строку которую нужно напечатать
 * Возвращаемое значение :  смотри возвращаемое значение в n3310lcd.h
 */
byte LcdStr ( LcdFontSize size, byte dataArray[], bool wrapLine, bool wrapDisplay  )
{
    byte tmpIdx=0;
    byte response;
    while( dataArray[ tmpIdx ] != '\0' )
    {
        // Выводим символ
        response = LcdChr( size, dataArray[ tmpIdx ] );
        // Не стоит волноваться если произойдет OUT_OF_BORDER,
        // строка будет печататься дальше из начала дисплея
        if (response == OK_WITH_WRAP && !wrapDisplay)
			return OK_WITH_WRAP;
        if(response == OUT_OF_BORDER)
			return OUT_OF_BORDER;
        // Увеличиваем указатель
        tmpIdx++;

		// если курсор уехал за границы строки (с первой буквы не проверяем, она, возможно с нулевой позиции)
		if (!wrapLine && ((LcdCacheIdx) % (LCD_X_RES ) == 0 ))
		{
			return OK_WITH_WRAP_LINE;
		}
    }
    return OK;
}



/*
 * Имя                   :  LcdFStr
 * Описание              :  Эта функция предназначена для печати строки которая хранится в Flash ROM
 * Аргумент(ы)           :  size    -> размер шрифта. Смотри enum в n3310.h
 *                          dataPtr -> указатель на строку которую нужно напечатать
 *							wrapLine -> если false, то если текст не влез в строку, то дальше не выводим
 *							wrapDisplay -> если false, то если текст не влез в дисплей, то дальше не выводим с начала дисплея
 * Возвращаемое значение :  смотри возвращаемое значение в n3310lcd.h
 * Пример                :  LcdFStr(FONT_1X, PSTR("Hello World"));
 *                          LcdFStr(FONT_1X, &name_of_string_as_array);
 */
byte LcdFStr ( LcdFontSize size, const byte *dataPtr, bool wrapLine, bool wrapDisplay )
{
    byte c;
    byte response;
    for ( c = pgm_read_byte( dataPtr ); c; ++dataPtr, c = pgm_read_byte( dataPtr ) )
    {
        // Выводим символ
        response = LcdChr( size, c );
		//if ((response == OK_WITH_WRAP_LINE || response == OK_WITH_WRAP) && !wrapLine)
			//return response; //OK_WITH_WRAP_LINE;
		if (response == OK_WITH_WRAP && !wrapDisplay)
			return OK_WITH_WRAP;
        if(response == OUT_OF_BORDER)
            return OUT_OF_BORDER;
			
		// если курсор уехал за границы строки (с первой буквы не проверяем, она, возможно с нулевой позиции)
		if (!wrapLine && ((LcdCacheIdx) % (LCD_X_RES ) == 0 ))
		{
			return OK_WITH_WRAP_LINE;
		}
    }
    return OK;
}



/*
 * Имя                   :  LcdPixel
 * Описание              :  Отображает пиксель по абсолютным координатам (x,y)
 * Аргумент(ы)           :  x,y  -> абсолютные координаты пикселя
 *                          mode -> Off, On или Xor. Смотри enum в n3310.h
 * Возвращаемое значение :  смотри возвращаемое значение в n3310lcd.h
 */
byte LcdPixel ( byte x, byte y, LcdPixelMode mode )
{
    int  index;
    byte  offset;
    byte  data;

    // Защита от выхода за пределы
    if ( x >= LCD_X_RES || y >= LCD_Y_RES) return OUT_OF_BORDER;

    // Пересчет индекса и смещения
    index = ( ( y / 8 ) * 84 ) + x;
    offset  = y - ( ( y / 8 ) * 8 );

    data = LcdCache[ index ];

    // Обработка битов

    // Режим PIXEL_OFF
    if ( mode == PIXEL_OFF )
    {
        data &= ( ~( 0x01 << offset ) );
    }
    // Режим PIXEL_ON
    else if ( mode == PIXEL_ON )
    {
        data |= ( 0x01 << offset );
    }
    // Режим PIXEL_XOR
    else if ( mode  == PIXEL_XOR )
    {
        data ^= ( 0x01 << offset );
    }

    // Окончательный результат копируем в кэш
    LcdCache[ index ] = data;

    if ( index < LoWaterMark )
    {
        // Обновляем нижнюю границу
        LoWaterMark = index;
    }

    if ( index > HiWaterMark )
    {
        // Обновляем верхнюю границу
        HiWaterMark = index;
    }
    return OK;
}



/*
 * Имя                   :  LcdLine
 * Описание              :  Рисует линию между двумя точками на дисплее (алгоритм Брезенхэма)
 * Аргумент(ы)           :  x1, y1  -> абсолютные координаты начала линии
 *                          x2, y2  -> абсолютные координаты конца линии
 *                          mode    -> Off, On или Xor. Смотри enum в n3310.h
 * Возвращаемое значение :  смотри возвращаемое значение в n3310lcd.h
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

    // dy отрицательное
    if ( dy < 0 )
    {
        dy    = -dy;
        stepy = -1;
    }
    else
    {
        stepy = 1;
    }

    // dx отрицательное
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

    // Рисуем начальную точку
    response = LcdPixel( x1, y1, mode );
    if(response)
        return response;

    // Рисуем следующие точки до конца
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

    // Установка флага изменений кэша
    UpdateLcd = TRUE;
    return OK;
}



/*
 * Имя                   :  LcdCircle
 * Описание              :  Рисует окружность (алгоритм Брезенхэма)
 * Аргумент(ы)           :  x, y   -> абсолютные координаты центра
 *                          radius -> радиус окружности
 *                          mode   -> Off, On или Xor. Смотри enum в n3310.h
 * Возвращаемое значение :  смотри возвращаемое значение в n3310lcd.h
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

    // Установка флага изменений кэша
    UpdateLcd = TRUE;
    return OK;
}




/*
 * Имя                   :  LcdRect
 * Описание              :  Рисует незакрашенный прямоугольник
 * Аргумент(ы)           :  x1    -> абсолютная координата x левого верхнего угла
 *                          y1    -> абсолютная координата y левого верхнего угла
 *                          x2    -> абсолютная координата x правого нижнего угла
 *                          y2    -> абсолютная координата y правого нижнего угла
 *                          mode  -> Off, On или Xor. Смотри enum в n3310.h
 * Возвращаемое значение :  смотри возвращаемое значение в n3310lcd.h
 */
byte LcdRect ( byte x1, byte y1, byte x2, byte y2, LcdPixelMode mode )
{
    byte tmpIdx;

    // Проверка границ
    if ( ( x1 >= LCD_X_RES) ||  ( x2 >= LCD_X_RES) || ( y1 >= LCD_Y_RES) || ( y2 >= LCD_Y_RES) )
        return OUT_OF_BORDER;

    if ( ( x2 > x1 ) && ( y2 > y1 ) )
    {
        // Рисуем горизонтальные линии
        for ( tmpIdx = x1; tmpIdx <= x2; tmpIdx++ )
        {
            LcdPixel( tmpIdx, y1, mode );
            LcdPixel( tmpIdx, y2, mode );
        }

        // Рисуем вертикальные линии
        for ( tmpIdx = y1; tmpIdx <= y2; tmpIdx++ )
        {
            LcdPixel( x1, tmpIdx, mode );
            LcdPixel( x2, tmpIdx, mode );
        }

        // Установка флага изменений кэша
        UpdateLcd = TRUE;
    }
	
    return OK;
}



/*
 * Имя                   :  LcdImage
 * Описание              :  Рисует картинку из массива сохраненного в Flash ROM
 * Аргумент(ы)           :  Указатель на массив картинки
 * Возвращаемое значение :  Нет
 */
void LcdImage ( const byte *imageData )
{
//    // Инициализация указателя кэша
//    LcdCacheIdx = 0;
//    // В пределах кэша
//    for ( LcdCacheIdx = 0; LcdCacheIdx < LCD_CACHE_SIZE; LcdCacheIdx++ )
//    {
//        // Копируем данные из массива в кэш
//        LcdCache[LcdCacheIdx] = pgm_read_byte( imageData++ );
//    }
    
    // Оптимизация от Jakub Lasinski (March 14 2009)
    memcpy_P( LcdCache, imageData, LCD_CACHE_SIZE );  // Тоже самое что и выше, но занимает меньше памяти и быстрее выполняется
    
    // Сброс указателей границ в максимальное значение
    LoWaterMark = 0;
    HiWaterMark = LCD_CACHE_SIZE - 1;

    // Установка флага изменений кэша
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
	Наличие пикселя в bitmap {Width, Height, {BYTES}}

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
	Вывод картинки из формата Gamebuino (https://github.com/Rodot/Gamebuino/blob/64de814fc8cd746fe66a7074b2c18960067aad1d/utility/Display.cpp#L495)
	{Width, Height, {BYTES}}
	BYTES - байты пикселей, из LCD Assistant подобных приложений указывать Byte Orientation - Horizontal, 8 pixels/byte
		пример:
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
					case ROTCCW: //90° counter-clockwise
					k = j;
					l = w - i - 1;
					break;
					case ROT180: //180°
					k = w - i - 1;
					l = h - j - 1;
					break;
					case ROTCW: //90° clockwise
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
	return 1; //write(str); // TODO: должно возвращать количество напечатанных символов
}