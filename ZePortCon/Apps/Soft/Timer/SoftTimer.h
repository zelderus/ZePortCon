/*
 * SoftTimer.h
 *
 * Created: 21.09.2017 16:50:10
 *  Author: kotenko_kg
 */ 


#ifndef SOFTTIMER_H_
#define SOFTTIMER_H_


#include "../../App.h"



class SoftTimer : public App
{
private:
	unsigned long ms;
	unsigned long ma;

public:
	SoftTimer() {}
	~SoftTimer() {}

	void begin()
	{
		ms = 0;
		ma = 0;
	}

	void loop()
	{
		KeyboardButton keyButtons = BTN_NONE;
		while(1)
		{
			keyButtons = zeos->keyboard.KeyboardReadButtonsCheckLast();
			if (keyButtons == BTN_BACK)
			{
				return;
			}

			zeos->display.Clear();

			// миллисекунды (1/1000)
			ms = millis();
			
			zeos->display.Goto(1, 1);
			zeos->display.Print(ms);
			// секунды
			zeos->display.Goto(1, 2);
			zeos->display.Print(ms/1000);
			// микросекунды (1/1000000)
			ma = micros();
			zeos->display.Goto(1, 3);
			zeos->display.Print(ma);

			zeos->display.Update();
		}
	}
};


#endif /* SOFTTIMER_H_ */