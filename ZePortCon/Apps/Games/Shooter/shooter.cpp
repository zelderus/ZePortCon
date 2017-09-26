/*
 * shooter.cpp
 *
 * Created: 25.09.2017 14:14:16
 *  Author: kotenko_kg
 */ 


 
#include "shooter.h"
#include <util/delay.h>
#include "data.h"
#include "../../../System/math.h"


#define __LCD_WIDTH		LCD_WIDTH
#define __LCD_HEIGHT	LCD_HEIGHT




#define __SHOOT_LOOPRESP_EXIT			0
#define __SHOOT_LOOPRESP_SCREEN_MAIN	1
#define __SHOOT_LOOPRESP_SCREEN_GAME	2





void GameShooter::begin()
{
	this->_x = __LCD_WIDTH/2;
	this->_y = __LCD_HEIGHT/2;

	camera_x = 0;
	camera_y = 0;


	//zeos->showCpuRam(); // вывод отладочной инфы
	zeos->setFrameRate(20);
}



void GameShooter::loop()
{
	unsigned int loopResponse = __SHOOT_LOOPRESP_EXIT;
	unsigned int (GameShooter::*currentLoop)(void) = &GameShooter::screenLogo;
	while(1)
	{
		// выполняем текущую сцену
		loopResponse = (this->*currentLoop)();
		// если выход из приложения
		if (loopResponse == __SHOOT_LOOPRESP_EXIT) return;
		// выбор сцены
		switch(loopResponse)
		{
			case __SHOOT_LOOPRESP_SCREEN_MAIN: currentLoop = &GameShooter::screenMain; break;
			case __SHOOT_LOOPRESP_SCREEN_GAME: currentLoop = &GameShooter::screenGame; break;
		}
	}
}



//
// logo
//
unsigned int GameShooter::screenLogo(void)
{
	zeos->display.Clear();
	zeos->display.DrawBitmapOnCenter(data_logo);
	zeos->display.Update();
	KeyboardButton keyButtons = BTN_NONE;
	while(1)
	{
		keyButtons = zeos->keyboard.KeyboardReadButtonsCheckLast();
		if (keyButtons == BTN_BACK) return __SHOOT_LOOPRESP_EXIT;
		else if (keyButtons != BTN_NONE) return __SHOOT_LOOPRESP_SCREEN_MAIN;
	}
}

//
// main
//
unsigned int GameShooter::screenMain(void)
{
	zeos->display.Clear();
	//int titleLenght = 8;
	//zeos->display.Goto((LCD_WIDTH-titleLenght)/2/8, 1);
	zeos->display.Goto(3, 1);
	zeos->display.FStr(data_txt_main_title);
	zeos->display.Goto(2, 3);
	zeos->display.FStr(data_txt_main_start);
	zeos->display.Goto(2, 4);
	zeos->display.FStr(data_txt_main_exit);
	zeos->display.Update();
	KeyboardButton keyButtons = BTN_NONE;
	while(1)
	{
		keyButtons = zeos->keyboard.KeyboardReadButtonsCheckLast();
		if ((keyButtons & BTN_BACK) | (keyButtons & BTN_A))
		{
			return __SHOOT_LOOPRESP_EXIT;
		}
		else if ((keyButtons & BTN_B) | (keyButtons & BTN_RIGHT))
		{
			return __SHOOT_LOOPRESP_SCREEN_GAME;
		}
	}
}




//
// игра
//
unsigned int GameShooter::screenGame(void)
{
	KeyboardButton keyButtons = BTN_NONE;

	int camera_v = 1;
	unsigned int camera_speed = 3;

	while(1)
	{
		if (zeos->update())
		{

			// buttons
			keyButtons = zeos->keyboard.KeyboardReadButtons(); //KeyboardReadButtonsCheckLast();
			if (keyButtons == BTN_BACK)
			{
				zeos->keyboard.KeyboardCurrentToLast();
				return __SHOOT_LOOPRESP_SCREEN_MAIN;
			}
			else if (keyButtons == BTN_UP)
			{
				//ball_vy--;
			}
			else if (keyButtons == BTN_DOWN)
			{
				//ball_vy++;
			}


			// camera
			camera_x += camera_v*camera_speed;
			//if (camera_x > 83) camera_v = -1;
			//if (camera_x < 0) camera_v = 1;
			if (camera_x > (WORLD_W * SPRITE_SIZE)-__LCD_WIDTH) camera_v = -1;
			if (camera_x < 0) camera_v = 1;

		
		

			// draw
			//zeos->display.DrawBitmap(10, 10, data_blockRight, ROTCW, NOFLIP);
			//zeos->display.DrawBitmap(10, 10, data_blockRight);
			drawWorld();
		
		}
	}
}







byte GameShooter::getTile(byte x, byte y)
{
	return pgm_read_byte(world + y*(WORLD_W) + x);
}
void GameShooter::drawWorld()
{
	for(byte y = max(0, (camera_y)/SPRITE_SIZE); y < min(WORLD_H, (camera_y+__LCD_HEIGHT)/SPRITE_SIZE+1); y++){
		for(byte x = max(0, (camera_x)/SPRITE_SIZE); x < min(WORLD_W, (camera_x+__LCD_WIDTH)/SPRITE_SIZE+1); x++){
			byte spriteID = getTile(x,y);
			int x_screen = x*SPRITE_SIZE - camera_x;
			int y_screen = y*SPRITE_SIZE - camera_y;
			zeos->display.DrawBitmap(x_screen, y_screen, sprites[spriteID]);
		}
	}
}





