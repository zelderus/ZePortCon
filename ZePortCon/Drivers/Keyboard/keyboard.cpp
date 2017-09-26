/*
 * keyboard.cpp
 *
 * Created: 18.09.2017 15:37:01
 *  Author: kotenko_kg
 */ 


 #include "keyboard.h"

#include <avr/io.h>
#include <util/delay.h>







#define   KEYBOARD_NUMBIT          8      // количество входов-бит ...1 регистр 8 входов


void Keyboard::KeyboardInit()
{
	// включаем нужные порты на выход (остальные не трогаем)
	KEYBOARD_WORK_DDR |= (1 << KEYBOARD_SHLD);
	KEYBOARD_WORK_DDR |= (1 << KEYBOARD_CLK);
	// включаем нужные порты на вход
	KEYBOARD_WORK_DDR &= ~(1 << KEYBOARD_BIT_IN);
}





const KeyboardButton Keyboard::_readKeyboard()
{
	byte __dataBit = 0;

	KEYBOARD_WORK_PORT &= ~(_BV(KEYBOARD_SHLD)); // 0
	_delay_us(300);
	KEYBOARD_WORK_PORT |= _BV(KEYBOARD_SHLD); // 1
	
	//for( unsigned int i=0; i<BIT; i++ )
	for( int i=KEYBOARD_NUMBIT-1; i>=0; --i )  // со сдвигово регистра биты идут с последней ножки/бита
	{
		_delay_us(100);

		if (KEYBOARD_WORK_PIN & (_BV(KEYBOARD_BIT_IN))) __dataBit |= 1 << i;
		//else __dataBit &= ~(0 << i); // там итак ноль
 
		KEYBOARD_WORK_PORT &= ~(_BV(KEYBOARD_CLK)); // 0  
		_delay_us(100);
		KEYBOARD_WORK_PORT |= _BV(KEYBOARD_CLK); // 1
	}
	return (KeyboardButton)__dataBit;
}




KeyboardButton __currentButtons = BTN_NONE;
const KeyboardButton Keyboard::KeyboardReadButtons()
{
	__currentButtons = _readKeyboard();
	return __currentButtons;
}

KeyboardButton __lastButtons = BTN_NONE;
const KeyboardButton Keyboard::KeyboardReadButtonsCheckLast()
{
	KeyboardButton buttons = KeyboardReadButtons();
	if (buttons != BTN_NONE && buttons != __lastButtons)
	{
		__lastButtons = buttons;
		return buttons;
	}
	__lastButtons = buttons;
	return BTN_NONE;
}

void Keyboard::KeyboardCurrentToLast()
{
	__lastButtons = __currentButtons;
}

