/*
 * settings.h
 *
 * Created: 22.09.2017 10:21:01
 *  Author: kotenko_kg
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_






/*************************

		клавиатура

**************************/
// биты, какая кнопка на какой пин сдвигово регистра
#define		BTN_UP_BIT					0b01000000 //0b00100000
#define		BTN_RIGHT_BIT				0b00100000 //0b00010000
#define		BTN_DOWN_BIT				0b00001000 //0b10000000
#define		BTN_LEFT_BIT				0b10000000 //0b01000000
#define		BTN_A_BIT					0b00000001 //0b00001000
#define		BTN_B_BIT					0b00000010
// подключение сдвигово регистра
#define		KEYBOARD_WORK_PORT			PORTA
#define		KEYBOARD_WORK_DDR			DDRA
#define		KEYBOARD_WORK_PIN			PINA
#define		KEYBOARD_CLK				0		// строб - сдвиг данных
#define		KEYBOARD_SHLD				1		// защёлкивание входов
#define		KEYBOARD_BIT_IN				2		// вход данных




/*************************

			LCD

**************************/
#define		LCD_WIDTH					84    // разрешение по горизонтали
#define		LCD_HEIGHT					48    // разрешение по вертикали
// закомментируйте эту директиву, если ваш дисплей оригинальный
//#define	CHINA_LCD
// Библиотека использует аппаратный SPI
#define		LCD_PORT					PORTB
#define		LCD_DDR						DDRB
// Распиновка порта
#define		LCD_DC_PIN					PB1
#define		LCD_CE_PIN					PB2
#define		SPI_MOSI_PIN				PB5   // SDIN дисплея обязательно подключаем к MOSI аппаратного SPI
#define		LCD_RST_PIN					PB4
#define		SPI_CLK_PIN					PB7   // SCLK дисплея обязательно подключаем к SCK аппаратного SPI









#endif /* SETTINGS_H_ */