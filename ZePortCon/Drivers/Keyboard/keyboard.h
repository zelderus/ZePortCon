/*
 * keyboard.h
 *
 * Created: 18.09.2017 15:37:11
 *  Author: kotenko_kg
 */ 


#ifndef KEYBOARD_H_
#define KEYBOARD_H_


#include "../../Types/defines.h"
#include "../../settings.h"




typedef enum
{
	BTN_NONE	= 0b00000000,
	BTN_UP		= BTN_UP_BIT,
	BTN_RIGHT	= BTN_RIGHT_BIT,
	BTN_DOWN	= BTN_DOWN_BIT,
	BTN_LEFT	= BTN_LEFT_BIT,
	BTN_A		= BTN_A_BIT,
	BTN_B		= BTN_B_BIT,

	BTN_BACK	= BTN_LEFT_BIT //BTN_A_BIT //TODO: ������� �� �������� �������� ������ (�� C)
} KeyboardButton;



class Keyboard
{
private:
	// �������� ������� ������� ������
	const KeyboardButton _readKeyboard();

public:
	// �������������
	void KeyboardInit();
	// �������� ������� ������� ������
	const KeyboardButton KeyboardReadButtons();
	// �������� ������� ������� ������ � ��������� (���� � ������� ��� ���� ������ ����� �� ������, �� ������ BTN_NONE - ����� ������ ����� � ����, ����� ����� ����������� ������������, �� �� ��� ���)
	const KeyboardButton KeyboardReadButtonsCheckLast();	
	// ������ ��������� ������� ������ (��� �������, ��� ������ �� ����������/�����, ������� ������������ 'KeyboardReadButtons', �������� �� ���������, ����� �� ��������� ������)
	void KeyboardCurrentToLast();
};




#endif /* KEYBOARD_H_ */