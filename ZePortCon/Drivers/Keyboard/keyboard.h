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

	BTN_BACK	= BTN_LEFT_BIT //BTN_A_BIT //TODO: вернуть на реальную привязку кнопки (на C)
} KeyboardButton;



class Keyboard
{
private:
	// получаем текущие нажатые кнопки
	const KeyboardButton _readKeyboard();

public:
	// инициазизация
	void KeyboardInit();
	// получаем текущие нажатые кнопки
	const KeyboardButton KeyboardReadButtons();
	// получаем текущие нажатые кнопки с проверкой (если в прошлый раз были нажаты такие же кнопки, то вернем BTN_NONE - такой подход важен в меню, когда нужно однократное реагирование, но НЕ для игр)
	const KeyboardButton KeyboardReadButtonsCheckLast();	
	// замена последней нажатой кнопки (как правило, при выходе из приложения/сцены, которое использовало 'KeyboardReadButtons', заменяем на последнее, чтобы не сработало дважды)
	void KeyboardCurrentToLast();
};




#endif /* KEYBOARD_H_ */