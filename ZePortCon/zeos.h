/*
 * zeos.h
 *
 * Created: 21.09.2017 13:26:09
 *  Author: kotenko_kg
 */ 


#ifndef ZEOS_H_
#define ZEOS_H_



#include <stdint.h> 
#include <util/delay.h>
#include "Drivers/Display/Display.h"
#include "Drivers/Timer/timer.h"
#include "Drivers/Keyboard/keyboard.h"



class Zeos
{
	protected:
		
	
	public:
		Keyboard keyboard;
		Display display;


		void showCpuRam();					// показывать отладочную информацию (кому надо приложению, должны вызвать в begin) - сбрасывается по дефолту перед запуском каждого приложения
		void setFrameRate(uint8_t fps);		// кадров в секунду, при использовании zeos->update() (кому надо приложению, должны вызвать в begin) - сбрасывается по дефолту перед запуском каждого приложения
		uint8_t getCpuLoad();
		uint16_t getFreeRam();
		
		void init();
		void begin();
		bool update();

		void delay_s(unsigned int s) { _delay_ms(s*1000); }
		void delay_ms(unsigned int s) { _delay_ms(s*1000); }

		// physics
		bool collidePointRect(int16_t x1, int16_t y1 ,int16_t x2 ,int16_t y2, int16_t w, int16_t h);
		bool collideRectRect(int16_t x1, int16_t y1, int16_t w1, int16_t h1 ,int16_t x2 ,int16_t y2, int16_t w2, int16_t h2);
		bool collideBitmapBitmap(int16_t x1, int16_t y1, const uint8_t* b1, int16_t x2, int16_t y2, const uint8_t* b2);


	private:
		bool _showCpuRam;

		void drawStartLogo();

		unsigned long frameCount;
		uint16_t frameDurationMicros;
		unsigned long frameStartMicros, frameEndMicros;

		uint8_t timePerFrame;
		uint32_t nextFrameMillis;
};




#endif /* ZEOS_H_ */