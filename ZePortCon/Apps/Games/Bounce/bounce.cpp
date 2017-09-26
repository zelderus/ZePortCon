/*
 * bounce.cpp
 *
 * Created: 21.09.2017 16:20:28
 *  Author: kotenko_kg
 */ 


 #include "bounce.h"

 #include <util/delay.h>



 void GameBounce::begin()
 {
	this->ball_x = LCD_X_RES/2; //set the horizontal position to the middle of the screen
	this->ball_y = LCD_Y_RES/2; //vertical position
	this->ball_vx = 3; //horizontal velocity
	this->ball_vy = 1; //vertical velocity
	this->ball_size = 6; //the size of the ball in number of pixels

	zeos->showCpuRam(); // вывод отладочной инфы
	zeos->setFrameRate(20);
 }

 void GameBounce::loop()
 {
	KeyboardButton keyButtons = BTN_NONE;

	

	while(1)
	{
		if (zeos->update())
		{

			keyButtons = zeos->keyboard.KeyboardReadButtons(); //KeyboardReadButtonsCheckLast();
			if (keyButtons == BTN_BACK)
			{
				return;
			}
			else if (keyButtons == BTN_UP)
			{
				ball_vy--;
			}
			else if (keyButtons == BTN_DOWN)
			{
				ball_vy++;
			}



			//add the speed of the ball to its position
			ball_x = ball_x + ball_vx;
			ball_y = ball_y + ball_vy;
			//check that the ball is not going out of the screen
			//if the ball is touching the left side of the screen
			if(ball_x < 0){
				//change the direction of the horizontal speed
				ball_vx = -ball_vx;
				ball_x = 0;
			}
			//if the ball is touching the right side
			if((ball_x + ball_size) > LCD_X_RES-1){
				ball_vx = -ball_vx;
				ball_x = LCD_X_RES-ball_size-1;
			}
			//if the ball is touching the top side
			if(ball_y < 0){
				ball_vy = -ball_vy;
				ball_y = 0;
			}
			//if the ball is touching the down side
			if((ball_y + ball_size) > LCD_Y_RES-1){
				ball_vy = -ball_vy;
				ball_y = LCD_Y_RES-ball_size-1;
			}
		
		
			//draw the ball on the screen
			//gb.display.fillRect(ball_x, ball_y, ball_size, ball_size);
			zeos->display.Rect(ball_x, ball_y, ball_x+ball_size, ball_y+ball_size, PIXEL_ON);
		}
	}
 }