/*
 * settings.h
 *
 * Created: 22.09.2017 10:21:01
 *  Author: kotenko_kg
 */ 


#ifndef SETTINGS_H_
#define SETTINGS_H_






/*************************

		����������

**************************/
// ����, ����� ������ �� ����� ��� �������� ��������
#define		BTN_UP_BIT					0b01000000 //0b00100000
#define		BTN_RIGHT_BIT				0b00100000 //0b00010000
#define		BTN_DOWN_BIT				0b00001000 //0b10000000
#define		BTN_LEFT_BIT				0b10000000 //0b01000000
#define		BTN_A_BIT					0b00000001 //0b00001000
#define		BTN_B_BIT					0b00000010
// ����������� �������� ��������
#define		KEYBOARD_WORK_PORT			PORTA
#define		KEYBOARD_WORK_DDR			DDRA
#define		KEYBOARD_WORK_PIN			PINA
#define		KEYBOARD_CLK				0		// ����� - ����� ������
#define		KEYBOARD_SHLD				1		// ������������ ������
#define		KEYBOARD_BIT_IN				2		// ���� ������




/*************************

			LCD

**************************/
#define		LCD_WIDTH					84    // ���������� �� �����������
#define		LCD_HEIGHT					48    // ���������� �� ���������
// ��������������� ��� ���������, ���� ��� ������� ������������
//#define	CHINA_LCD
// ���������� ���������� ���������� SPI
#define		LCD_PORT					PORTB
#define		LCD_DDR						DDRB
// ���������� �����
#define		LCD_DC_PIN					PB1
#define		LCD_CE_PIN					PB2
#define		SPI_MOSI_PIN				PB5   // SDIN ������� ����������� ���������� � MOSI ����������� SPI
#define		LCD_RST_PIN					PB4
#define		SPI_CLK_PIN					PB7   // SCLK ������� ����������� ���������� � SCK ����������� SPI









#endif /* SETTINGS_H_ */