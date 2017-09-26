/*
 * Game.h
 *
 * Created: 21.09.2017 16:21:00
 *  Author: kotenko_kg
 */ 


#ifndef GAME_H_
#define GAME_H_

#include <avr/pgmspace.h>
#include "../zeos.h"


const byte _errStr[] PROGMEM = "NO LOOP";


class App
{
	protected:
		Zeos* zeos;

	public:
		App() {}
		virtual ~App() {}

		void init(Zeos* zeos)
		{
			this->zeos = zeos;
		}
		virtual	void begin() {}
		virtual void loop() 
		{
			zeos->display.Clear();
			zeos->display.Goto(0, 5);
			zeos->display.FStr(_errStr);
			zeos->display.Update();

			KeyboardButton keyButtons = BTN_NONE;
			while(1)
			{
				keyButtons = zeos->keyboard.KeyboardReadButtonsCheckLast();
				if (keyButtons == BTN_BACK)
				{
					return;
				}
			}
		}

};




#endif /* GAME_H_ */