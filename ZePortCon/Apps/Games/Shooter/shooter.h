/*
 * shooter.h
 *
 * Created: 25.09.2017 14:14:26
 *  Author: kotenko_kg
 */ 


#ifndef SHOOTER_H_
#define SHOOTER_H_


#include "../../App.h"



class GameShooter : public App
{
private:
	int _x;
	int _y;

	int camera_x ;
	int camera_y ;


	byte getTile(byte x, byte y);
	void drawWorld();


	unsigned int screenLogo(void);
	unsigned int screenMain(void);
	unsigned int screenGame(void);

public:
	~GameShooter() {}
	GameShooter() {}

	void begin();
	void loop();
};





#endif /* SHOOTER_H_ */