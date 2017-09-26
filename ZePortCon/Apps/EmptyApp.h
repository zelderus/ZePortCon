/*
 * EmptyApp.h
 *
 * Created: 22.09.2017 12:48:49
 *  Author: kotenko_kg
 */ 


#ifndef EMPTYAPP_H_
#define EMPTYAPP_H_



#include <avr/pgmspace.h>
#include "App.h"



const byte _emptyAppText[] PROGMEM = "Ќе указан AppId";




class EmptyApp : public App
{
	private:

	public:
	EmptyApp() {}
	~EmptyApp() {}

	void begin()
	{

	}

	void loop()
	{
		// тут хватит и один раз нарисовать
		zeos->display.DrawErrorText(_emptyAppText, 0);

		KeyboardButton keyButtons = BTN_NONE;
		while(1)
		{
			keyButtons = zeos->keyboard.KeyboardReadButtonsCheckLast();
			if (keyButtons == BTN_BACK)
			{
				return;
			}

			//zeos->display.Clear();
			//zeos->display.Goto(1, 2);
			//zeos->display.FStr(_emptyAppText);
			//zeos->display.Update();
		}
	}
};



#endif /* EMPTYAPP_H_ */