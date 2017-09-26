/*
 * AppManager.h
 *
 * Created: 22.09.2017 10:51:31
 *  Author: kotenko_kg
 */ 


#ifndef APPMANAGER_H_
#define APPMANAGER_H_


#include <stdint.h> 
#include "../Apps/App.h"
#include "../Apps/EmptyApp.h"

#include "../Apps/Games/Bounce/bounce.h"
#include "../Apps/Games/Shooter/shooter.h"

#include "../Apps/Soft/Timer/SoftTimer.h"





class AppManager
{
private:
	App* _currentApp;


public:

	void init()
	{
		_currentApp = 0;

	}


	/*
		ѕриложени€ и их ID.
		что-бы убрать приложение/игру из пам€ти, достаточно закомментировать тут строку (компил€тор сам поймет, что не используетс€ и не будет писать ее данные)
	*/
	App* getApp(unsigned int appId)
	{
		switch(appId)
		{
			// games
			case 0: { _currentApp = new EmptyApp(); break; }
			case 1: { _currentApp = new GameBounce(); break; }
			case 2: { _currentApp = new GameShooter(); break; }
			// soft
			case 101: { _currentApp = new SoftTimer(); break; }
		}
		return _currentApp;
	}

	void clearApp()
	{
		//if (_currentApp != 0) delete _currentApp;
		delete _currentApp;
		_currentApp = 0;
	}

};





#endif /* APPMANAGER_H_ */