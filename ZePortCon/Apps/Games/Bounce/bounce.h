/*
 * bounce.h
 *
 * Created: 21.09.2017 16:20:37
 *  Author: kotenko_kg
 */ 


#ifndef BOUNCE_H_
#define BOUNCE_H_


#include "../../App.h"



class GameBounce : public App
{
	private:
		int ball_x; //set the horizontal position to the middle of the screen
		int ball_y; //vertical position
		int ball_vx; //horizontal velocity
		int ball_vy; //vertical velocity
		int ball_size; //the size of the ball in number of pixels

	public:
		~GameBounce() {}
		GameBounce() {}

		void begin();
		void loop();
};



#endif /* BOUNCE_H_ */