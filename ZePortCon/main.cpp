/*
 * ZePortCon.cpp
 *
 * Created: 16.09.2017 17:21:31
 * Author : Kirill
 */ 




 //#define F_CPU (1200000UL)
 #ifndef F_CPU
	#error "F_CPU is not defined in 'Project -> Properties -> Toolchain -> C++Compiler -> Symbols'"
	#define F_CPU 1000000UL
 #endif
 
 

#include <avr/io.h>
//#include <stdio.h>
#include <avr/pgmspace.h>
//#include <avr/eeprom.h>


#include <stdlib.h> // for malloc and free
void* operator new(size_t size) { return malloc(size); }
void operator delete(void* ptr) { free(ptr); }




#include "Types/defines.h"
#include "zeos.h"
#include "Managers/MenuManager.h"
#include "Managers/AppManager.h"

#include "Apps/App.h"
#include "Apps/Games/Bounce/bounce.h"
#include "Apps/Soft/Timer/SoftTimer.h"





const byte _loadingText[] PROGMEM = "ЗАГРУЗКА..";


#if DEBUG
// RAM
byte _startAddr[] = "СТАРТ";
#endif





// Globals
Zeos zeos;
MenuManager menu;
AppManager apps;







int main(void)
{
#if DEBUG
	DDRB = 0xFF;
	PORTB = 0x99;
	DDRC = 0xFF;
	PORTC = 0x99;
	PORTC = *_startAddr; // отметка о начале в RAM (чтобы визуально видеть в отладчике)
#endif


	// основное (вся переферия)
	zeos.init();

	// меню
	MenuManager* rMenu = &menu;
	rMenu->Init(&zeos);
	// приложения 
	apps.init();


	// рисуем первое меню
	rMenu->DrawCurrentMenu();
	



	KeyboardButton keyButtons = BTN_NONE;
	while (1)
	{
		
		// слушаем клаву
		keyButtons = zeos.keyboard.KeyboardReadButtonsCheckLast();



		if (keyButtons != BTN_NONE)
		{
			MenuManagerResponse menuResponse = MENU_DRAW;
			switch(keyButtons)
			{
				case BTN_NONE: break;
				case BTN_UP: menuResponse = rMenu->SetCursorToPrev(); break;
				case BTN_DOWN: menuResponse = rMenu->SetCursorToNext(); break;
				case BTN_LEFT: menuResponse = rMenu->SetParentMenu(); break;
				case BTN_RIGHT: menuResponse = rMenu->RunCurrentMenu(); break;
				case BTN_A: menuResponse = rMenu->SetParentMenu(); break;
				case BTN_B: menuResponse = rMenu->RunCurrentMenu(); break;
			}
			// запуск программы
			if (menuResponse == APP_RUN)
			{
				// draw loading
				zeos.display.DrawCenterText(_loadingText, 2);

				// получаем приложение
				App* app = apps.getApp(rMenu->GetMenuByCursor()->AppId);
				// запускаем
				zeos.begin();
				app->init(&zeos);
				app->begin();
				zeos.display.Clear(); // очищаем надпись загрузка (если нужен логотип игры, то рисовать его в основной ветке 'loop')
				app->loop();
				// после цикла приложения - очищаем память
				app = 0;
				apps.clearApp();
				// восстанавливаем нажатие кнопок (чтобы не нажали в прошлом приложении, кладем эти значения как последние, и они не отрабоют повторно)
				zeos.keyboard.KeyboardCurrentToLast();
			}
			// рисуем меню
			rMenu->DrawCurrentMenu();
		}


		//PORTC = 0x55;//pgm_read_byte(itmRef->Title);
	}
	
}
