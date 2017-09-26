/*
 * zeos.cpp
 *
 * Created: 21.09.2017 13:25:55
 *  Author: kotenko_kg
 */ 


#include "zeos.h"
#include <avr/pgmspace.h>


const byte _startMarkLogo[] PROGMEM = { 0x10, 0x00 }; //">";
const byte _cpuStr[] PROGMEM = "CPU: ";
const byte _ramStr[] PROGMEM = "RAM: ";


void Zeos::init()
{
	timePerFrame = 50;
	frameEndMicros = 1;
	_showCpuRam = false;

	TimerInit();
	//apiLcdInit();
	keyboard.KeyboardInit();
	display.Init();
	display.Contrast(0x3c);
	
	drawStartLogo();
	
	sei();
}

void Zeos::drawStartLogo()
{
	display.Clear();
	display.Goto(0,0);
	display.FStr(_startMarkLogo);
	display.Update();
}


void Zeos::begin() // выполняется перед каждым запуском нового приложения
{
	_showCpuRam = false;
	timePerFrame = 50;
	//nextFrameMillis = 0;
	//frameCount = 0;
	frameEndMicros = 1;
	//startMenuTimer = 255;
}



bool Zeos::update()
{
	if (((nextFrameMillis - millis()) > timePerFrame) && frameEndMicros) { //if time to render a new frame is reached and the frame end has ran once
		nextFrameMillis = millis() + timePerFrame;
		frameCount++;

		frameEndMicros = 0;
		frameStartMicros = micros();

		//backlight.update();
		//buttons.update();
		//battery.update();

		return true;

		} else {
		if (!frameEndMicros) { //runs once at the end of the frame
			
			////increase volume shortcut : hold C + press B
			//if(buttons.repeat(BTN_C, 1) && buttons.pressed(BTN_B)){
				//sound.setVolume(sound.getVolume() + 1);
				//popup(F("\027+\026 \23\24"), 60);
				//sound.playTick();
			//}
			////flash loader shortcut : hold C + A
			//if(buttons.repeat(BTN_C, 1) && buttons.pressed(BTN_A)){
				//popup(F("\027+\025 \020 LOADER"), 60);
			//}
			//if(buttons.repeat(BTN_C, 1) && buttons.held(BTN_A,40)){
				//changeGame();
			//}
			
			//sound.updateTrack();
			//sound.updatePattern();
			//sound.updateNote();
			//updatePopup();
			//displayBattery();
			
			//display.update(); //send the buffer to the screen
			//if(!display.persistence)
			//display.clear(); //clear the buffer
			LcdUpdate();
			LcdClear();


			frameEndMicros = micros(); //measure the frame's end time
			frameDurationMicros = frameEndMicros - frameStartMicros;

			//            display.setCursor(0, 32);
			//            display.print("CPU:");
			//            display.print(frameDurationMicros / timePerFrame);
			//            display.println("/1000");
			//            display.print("RAM:");
			//            display.print(2048 - freeRam());
			//            display.println("/2048");

			if (_showCpuRam)
			{
				display.Goto(0, 0);
				display.FStr(_cpuStr);
				display.Print(getCpuLoad());
				display.Goto(0, 1);
				display.FStr(_ramStr);
				display.Print(getFreeRam());
			}
		}
		return false;
	}
}



void Zeos::showCpuRam() {
	this->_showCpuRam = true;
}

void Zeos::setFrameRate(uint8_t fps) {
	timePerFrame = 1000 / fps;
	//sound.prescaler = fps / 20;
	//sound.prescaler = max(1, sound.prescaler);
}
uint8_t Zeos::getCpuLoad(){
	return(frameDurationMicros/(10*timePerFrame));
}
uint16_t Zeos::getFreeRam() {
	//from http://www.controllerprojects.com/2011/05/23/determining-sram-usage-on-arduino/
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}




/*
	Physics
*/
bool Zeos::collidePointRect(int16_t x1, int16_t y1 ,int16_t x2 ,int16_t y2, int16_t w, int16_t h)
{
	if((x1>=x2)&&(x1<x2+w))
		if((y1>=y2)&&(y1<y2+h))
			return true;
	return false;
}

bool Zeos::collideRectRect(int16_t x1, int16_t y1, int16_t w1, int16_t h1 ,int16_t x2 ,int16_t y2, int16_t w2, int16_t h2)
{
	return !( x2   >=  x1+w1  ||
			x2+w2  <=  x1     ||
			y2     >=  y1+h1  ||
			y2+h2  <=  y1     );
}

bool Zeos::collideBitmapBitmap(int16_t x1, int16_t y1, const uint8_t* b1, int16_t x2, int16_t y2, const uint8_t* b2)
{
	int16_t w1 = pgm_read_byte(b1);
	int16_t h1 = pgm_read_byte(b1 + 1);
	int16_t w2 = pgm_read_byte(b2);
	int16_t h2 = pgm_read_byte(b2 + 1);

	if(collideRectRect(x1, y1, w1, h1, x2, y2, w2, h2) == false){
		return false;
	}
	
	int16_t xmin = (x1>=x2)? 0 : x2-x1;
	int16_t ymin = (y1>=y2)? 0 : y2-y1;
	int16_t xmax = (x1+w1>=x2+w2)? x2+w2-x1 : w1;
	int16_t ymax = (y1+h1>=y2+h2)? y2+h2-y1 : h1;
	for(uint8_t y = ymin; y < ymax; y++){
		for(uint8_t x = xmin; x < xmax; x++){
			if(display.GetBitmapPixel(b1, x, y) && display.GetBitmapPixel(b2, x1+x-x2, y1+y-y2)){
				return true;
			}
		}
	}
	return false;
}