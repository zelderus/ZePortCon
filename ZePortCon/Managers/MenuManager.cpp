/*
 * MenuManager.cpp
 *
 * Created: 13.09.2017 11:46:12
 *  Author: kotenko_kg
 */ 


#include "MenuManager.h"

#include <avr/pgmspace.h>
//#include <avr/io.h>


// Code FLASH (компилятор разместит его в общей памяти данных/кода)
const byte menuZeroCh[] PROGMEM = { 0 };
const byte menuTitleMain[] PROGMEM = "ZeOS";		const byte menuTitleMain_ch[] PROGMEM = { 1, 4, 7, 9, 10, 11, 0 };	// дети всегда кончаются нулем
const byte menuTitle1[] PROGMEM = "1. Настройки";		const byte menuTitle1_ch[] PROGMEM = { 2, 3, 0 };
const byte menuTitle1_1[] PROGMEM = "1.1. Задача 1";
const byte menuTitle1_2[] PROGMEM = "1.2. Задача 2";
const byte menuTitle2[] PROGMEM = "2. Игры";		const byte menuTitle2_ch[] PROGMEM = { 5,6, 0 };
const byte menuTitle2_1[] PROGMEM = "Bounce";
const byte menuTitle2_2[] PROGMEM = "Shooter";
const byte menuTitle3[] PROGMEM = "3. Софт";		const byte menuTitle3_ch[] PROGMEM = { 8, 0 };
const byte menuTitle3_1[] PROGMEM = "Time test";
const byte menuTitle4[] PROGMEM = "4. Еще чего-то много текста";
const byte menuTitle5[] PROGMEM = "5. Еще чего-то";
const byte menuTitle6[] PROGMEM = "6. Еще чего-то";


void MenuManager::Init(Zeos* zeos)
{
	this->id = 1983;
	this->_itmId = 0;
	this->_itmCursorId = 0;
	this->_isCursorBack = false;
	this->_cursorNum = 1;
	this->debugByte = 0;
	this->zeos = zeos;
	

	// main
	this->_items[0] = MenuItem(menuTitleMain, -1, menuTitleMain_ch);

	// menu 1 (настройки)
	this->_items[1] = MenuItem(menuTitle1, 0, menuTitle1_ch);
		this->_items[2] = MenuItem(menuTitle1_1, 1, menuZeroCh, 0);
		this->_items[3] = MenuItem(menuTitle1_2, 1, menuZeroCh, 0);

	// menu 2 (игры)
	this->_items[4] = MenuItem(menuTitle2, 0, menuTitle2_ch);
		this->_items[5] = MenuItem(menuTitle2_1, 4, menuZeroCh, 1);
		this->_items[6] = MenuItem(menuTitle2_2, 4, menuZeroCh, 2);
	// menu 3 (софт)
	this->_items[7] = MenuItem(menuTitle3, 0, menuTitle3_ch);
		this->_items[8] = MenuItem(menuTitle3_1, 6, menuZeroCh, 101);

	// menu 4
	this->_items[9] = MenuItem(menuTitle4, 0, menuZeroCh);
	// menu 5
	this->_items[10] = MenuItem(menuTitle5, 0, menuZeroCh);
	// menu 6
	this->_items[11] = MenuItem(menuTitle6, 0, menuZeroCh);
	

	this->SetMenu(0);
}




MenuManagerResponse MenuManager::RunCurrentMenu()
{
	// если мы на back, то возврат к родителю
	if (this->IsCursorBack()) { this->SetParentMenu(); return MENU_DRAW; }
	// если есть дети у меню, на котором курсор, то идем к детям
	const MenuItem* rItm = this->GetMenuByCursor();
	byte firstId = pgm_read_byte(rItm->Children);
	if (firstId > 0) 
	{ 
		// запоминаем на кого указывали в последний раз из детей
		this->GetCurrentMenu()->LastCursorNum = this->_cursorNum;
		// заходим в текущего, на ком курсор
		this->SetMenu(GetMenuCursorId());
		return MENU_DRAW; 
	}
	// запуск программы
	return APP_RUN;
}


MenuManagerResponse MenuManager::SetParentMenu()
{
	const int parentId = this->GetCurrentMenu()->ParentId;
	if (parentId >= 0) this->SetMenu(parentId);
	return MENU_DRAW;
}
void MenuManager::SetMenu(byte menuId)
{
	if (menuId < 0) menuId = 0;
	this->_itmId = menuId;
	this->_itmRef = &this->_items[menuId];
	this->_isCursorBack = false;
	// выставляем курсор на первого ребенка (если есть)
	volatile MenuItem* rItm = this->GetCurrentMenu();
	this->SetCursorToChildrenByNum(this->_itmId, rItm->LastCursorNum);
}
bool MenuManager::IsCursorBack() const { return this->_isCursorBack; }
volatile MenuItem* MenuManager::GetCurrentMenu() const { return this->_itmRef; }
byte MenuManager::GetMenuCursorId() const { return this->_itmCursorId; }
const MenuItem* MenuManager::GetMenuByCursor() { return this->GetMenuById(this->_itmCursorId); }
const MenuItem* MenuManager::GetMenuById(byte menuId) const { return &this->_items[menuId]; }

void MenuManager::SetCursorToMenu(byte menuId, unsigned int cursorNum)
{
	this->_itmCursorId = 0;
	if (menuId <= 0) return;
	this->_itmCursorId = menuId;
	this->_isCursorBack = false; // сбрасываем флаг back
	if (cursorNum <= 0) return;
	this->_cursorNum = cursorNum;
}
void MenuManager::SetCursorToChildrenByNum(byte menuId, unsigned int num)
{
	int childId = this->GetMenuIdFromChildrenByNum(menuId, num);
	if (childId <= 0) return; // оставляем прежний
	this->SetCursorToMenu(childId, num);
}
unsigned int MenuManager::GetMenuIdFromChildrenByNum(byte menuId, unsigned int num)
{
	if (menuId < 0 || num <= 0) return 0;
	const MenuItem* rItm = this->GetMenuById(menuId);
	// дети
	unsigned int childId = 0;
	byte firstId = pgm_read_byte(rItm->Children);
	if (firstId > 0) // иначе нет детей
	{
		const byte* r = rItm->Children; // чтобы не смещать реальный указатель
		// из строки достаем ID нужных меню (лежит в PROGMEM)
		unsigned int step = 1;
		byte id = firstId; //pgm_read_byte(r);
		while (id)
		{
			if (step == num) // если последовательность ребенка совпадает с запросом
			{
				return id;
			}
			id = pgm_read_byte(++r);
			step++;
		}
	}
	return childId;
}
MenuManagerResponse MenuManager::SetCursorToPrev()
{
	if (this->_cursorNum < 1) return MENU_DRAW;
	if (this->_cursorNum == 1) // попытка на back
	{
		volatile MenuItem* rItm = this->GetCurrentMenu(); // если есть родительский, значит есть и back
		if (rItm->ParentId >= 0) 
		{
			this->_isCursorBack = true; // ставим флаг back
			this->_cursorNum = 0;
		}
		return MENU_DRAW;
	}
	this->SetCursorToChildrenByNum(this->_itmId, this->_cursorNum-1);
	return MENU_DRAW;
}
MenuManagerResponse MenuManager::SetCursorToNext()
{
	this->SetCursorToChildrenByNum(this->_itmId, this->_cursorNum+1);
	return MENU_DRAW;
}




/*

	DRAW

*/


void MenuManager::DrawCurrentMenu() const
{
	zeos->display.Clear();
	//menu->ShowCurrentMenu();

	unsigned int totalDisplayRows = 6;


	
	volatile MenuItem* rItm = GetCurrentMenu();
	byte menuByCursor = GetMenuCursorId(); // меню, на котором курсор
	unsigned int cursorNum = this->_cursorNum; // на какой по счету указываем (для смещения при скролле)


	// название
	DrawMenuTitle(rItm);
	totalDisplayRows--;


	// дети
	byte firstId = pgm_read_byte(rItm->Children);
	if (firstId > 0)
	{
		unsigned int rowNum = 1; // строка на дисплее, с которой будут выводиться дети (0 занял заголовок)
		bool hasMarked = false;
		// back menu
		if (rItm->ParentId >= 0)
		{
			hasMarked = IsCursorBack();
			DrawMenuBack(hasMarked);
			rowNum++;
			cursorNum = 1; // с первого пункта выводим (для смещения)
			totalDisplayRows--;
		}
		
		unsigned int totalChildCount = 0;
		// 1. первый проход - считаем сколько всего детей и прочая подготовка
		const byte* r = rItm->Children; // чтобы не смещать реальный указатель
		byte id = pgm_read_byte(r); // из строки достаем ID нужных меню (лежит в PROGMEM)
		while (id)
		{
			totalChildCount++;
			id = pgm_read_byte(++r);
		}
		// считаем с какого пунка по счету выводить (чтобы выбранный попал примерно в центр дисплея)
		unsigned int ofs = 2; // смещаемся на столько строк выше, чтобы попасть в центр (из всего ~5 свободных строк)
		int pos = cursorNum - ofs;
		if (pos <= 0) pos = 1;
		else
		{
			// если после текущей осталось совсем мало (не заполняем весь дисплей), то смещаемся еще
			int dif = totalChildCount - pos; // сколько осталось еще пунктов
			int emptyPlaces = totalDisplayRows - (dif + 1); // сколько осталось пустых строк
			pos -= emptyPlaces > 0 ? emptyPlaces : 0; // смешаемся чуть выше, для заполнения всего дисплея
		}

		unsigned int displayRowsForChildren = totalDisplayRows;
		// 2. второй проход - вывод с учетом диапазона
		int currItemRow = 1; // какой по счету пункт меню идет (начиная с 1)
		const byte* rc = rItm->Children; // чтобы не смещать реальный указатель
		id = pgm_read_byte(rc); // из строки достаем ID нужных меню (лежит в PROGMEM)
		while (id)
		{
			// строка пункта меню
			if (totalDisplayRows > 0 && pos <= currItemRow ) // рисуем, если не вышли за рамки дисплея и если с этого пункта рисуем
			{
				DrawMenuLine(GetMenuById(id), (id == menuByCursor && !hasMarked), rowNum++); // подсвечиваем текущий маркер, если не было подсвечено ранее back
				totalDisplayRows--;
			}
			currItemRow++;
			id = pgm_read_byte(++rc);
		}

		// TODO: рисуем скролл (если количество пунктов большое и не влезло в экран) - еще учесть back
		if (displayRowsForChildren < totalChildCount)
		{
			// всего детей - totalChildCount
			// текущий выбранный - cursorNum (начиная с 1)

			//zeos->display.SingleBar(82, 47, 39, 2, PIXEL_ON);
			//zeos->display.Line(83, 17, 83, 20, PIXEL_OFF);
		}
		

	}
	//else
	//{
	//LcdGotoXYFont(0, 3);
	//LcdFStr(FONT_1X, noTitle);
	//}
	
	// сразу и рисуем
	zeos->display.Update();
}


void MenuManager::DrawMenuTitle(volatile MenuItem* itmRef) const
{
	// вывод названия
	zeos->display.Goto(0, 0);
	zeos->display.FStr(itmRef->Title, false, false);
	// TODO: разделитель
}
void MenuManager::DrawMenuBack(bool isSelected) const
{
	zeos->display.Goto(0, 1);
	// отметка, текущего меню
	zeos->display.FStr(isSelected ? currentMenuMark : currentMenuNotMark);
	// вывод названия
	zeos->display.FStr(menuBackTitle, false, false);
}
byte MenuManager::DrawMenuLine(const MenuItem* itmRef, bool isSelected, unsigned int rowNum) const
{
	zeos->display.Goto(0,rowNum);
	// отметка, текущего меню
	zeos->display.FStr(isSelected ? currentMenuMark : currentMenuNotMark);
	// вывод названия
	return zeos->display.FStr(FONT_1X, itmRef->Title, false, false);
}
