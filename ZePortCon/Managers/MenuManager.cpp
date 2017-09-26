/*
 * MenuManager.cpp
 *
 * Created: 13.09.2017 11:46:12
 *  Author: kotenko_kg
 */ 


#include "MenuManager.h"

#include <avr/pgmspace.h>
//#include <avr/io.h>


// Code FLASH (���������� ��������� ��� � ����� ������ ������/����)
const byte menuZeroCh[] PROGMEM = { 0 };
const byte menuTitleMain[] PROGMEM = "ZeOS";		const byte menuTitleMain_ch[] PROGMEM = { 1, 4, 7, 9, 10, 11, 0 };	// ���� ������ ��������� �����
const byte menuTitle1[] PROGMEM = "1. ���������";		const byte menuTitle1_ch[] PROGMEM = { 2, 3, 0 };
const byte menuTitle1_1[] PROGMEM = "1.1. ������ 1";
const byte menuTitle1_2[] PROGMEM = "1.2. ������ 2";
const byte menuTitle2[] PROGMEM = "2. ����";		const byte menuTitle2_ch[] PROGMEM = { 5,6, 0 };
const byte menuTitle2_1[] PROGMEM = "Bounce";
const byte menuTitle2_2[] PROGMEM = "Shooter";
const byte menuTitle3[] PROGMEM = "3. ����";		const byte menuTitle3_ch[] PROGMEM = { 8, 0 };
const byte menuTitle3_1[] PROGMEM = "Time test";
const byte menuTitle4[] PROGMEM = "4. ��� ����-�� ����� ������";
const byte menuTitle5[] PROGMEM = "5. ��� ����-��";
const byte menuTitle6[] PROGMEM = "6. ��� ����-��";


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

	// menu 1 (���������)
	this->_items[1] = MenuItem(menuTitle1, 0, menuTitle1_ch);
		this->_items[2] = MenuItem(menuTitle1_1, 1, menuZeroCh, 0);
		this->_items[3] = MenuItem(menuTitle1_2, 1, menuZeroCh, 0);

	// menu 2 (����)
	this->_items[4] = MenuItem(menuTitle2, 0, menuTitle2_ch);
		this->_items[5] = MenuItem(menuTitle2_1, 4, menuZeroCh, 1);
		this->_items[6] = MenuItem(menuTitle2_2, 4, menuZeroCh, 2);
	// menu 3 (����)
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
	// ���� �� �� back, �� ������� � ��������
	if (this->IsCursorBack()) { this->SetParentMenu(); return MENU_DRAW; }
	// ���� ���� ���� � ����, �� ������� ������, �� ���� � �����
	const MenuItem* rItm = this->GetMenuByCursor();
	byte firstId = pgm_read_byte(rItm->Children);
	if (firstId > 0) 
	{ 
		// ���������� �� ���� ��������� � ��������� ��� �� �����
		this->GetCurrentMenu()->LastCursorNum = this->_cursorNum;
		// ������� � ��������, �� ��� ������
		this->SetMenu(GetMenuCursorId());
		return MENU_DRAW; 
	}
	// ������ ���������
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
	// ���������� ������ �� ������� ������� (���� ����)
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
	this->_isCursorBack = false; // ���������� ���� back
	if (cursorNum <= 0) return;
	this->_cursorNum = cursorNum;
}
void MenuManager::SetCursorToChildrenByNum(byte menuId, unsigned int num)
{
	int childId = this->GetMenuIdFromChildrenByNum(menuId, num);
	if (childId <= 0) return; // ��������� �������
	this->SetCursorToMenu(childId, num);
}
unsigned int MenuManager::GetMenuIdFromChildrenByNum(byte menuId, unsigned int num)
{
	if (menuId < 0 || num <= 0) return 0;
	const MenuItem* rItm = this->GetMenuById(menuId);
	// ����
	unsigned int childId = 0;
	byte firstId = pgm_read_byte(rItm->Children);
	if (firstId > 0) // ����� ��� �����
	{
		const byte* r = rItm->Children; // ����� �� ������� �������� ���������
		// �� ������ ������� ID ������ ���� (����� � PROGMEM)
		unsigned int step = 1;
		byte id = firstId; //pgm_read_byte(r);
		while (id)
		{
			if (step == num) // ���� ������������������ ������� ��������� � ��������
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
	if (this->_cursorNum == 1) // ������� �� back
	{
		volatile MenuItem* rItm = this->GetCurrentMenu(); // ���� ���� ������������, ������ ���� � back
		if (rItm->ParentId >= 0) 
		{
			this->_isCursorBack = true; // ������ ���� back
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
	byte menuByCursor = GetMenuCursorId(); // ����, �� ������� ������
	unsigned int cursorNum = this->_cursorNum; // �� ����� �� ����� ��������� (��� �������� ��� �������)


	// ��������
	DrawMenuTitle(rItm);
	totalDisplayRows--;


	// ����
	byte firstId = pgm_read_byte(rItm->Children);
	if (firstId > 0)
	{
		unsigned int rowNum = 1; // ������ �� �������, � ������� ����� ���������� ���� (0 ����� ���������)
		bool hasMarked = false;
		// back menu
		if (rItm->ParentId >= 0)
		{
			hasMarked = IsCursorBack();
			DrawMenuBack(hasMarked);
			rowNum++;
			cursorNum = 1; // � ������� ������ ������� (��� ��������)
			totalDisplayRows--;
		}
		
		unsigned int totalChildCount = 0;
		// 1. ������ ������ - ������� ������� ����� ����� � ������ ����������
		const byte* r = rItm->Children; // ����� �� ������� �������� ���������
		byte id = pgm_read_byte(r); // �� ������ ������� ID ������ ���� (����� � PROGMEM)
		while (id)
		{
			totalChildCount++;
			id = pgm_read_byte(++r);
		}
		// ������� � ������ ����� �� ����� �������� (����� ��������� ����� �������� � ����� �������)
		unsigned int ofs = 2; // ��������� �� ������� ����� ����, ����� ������� � ����� (�� ����� ~5 ��������� �����)
		int pos = cursorNum - ofs;
		if (pos <= 0) pos = 1;
		else
		{
			// ���� ����� ������� �������� ������ ���� (�� ��������� ���� �������), �� ��������� ���
			int dif = totalChildCount - pos; // ������� �������� ��� �������
			int emptyPlaces = totalDisplayRows - (dif + 1); // ������� �������� ������ �����
			pos -= emptyPlaces > 0 ? emptyPlaces : 0; // ��������� ���� ����, ��� ���������� ����� �������
		}

		unsigned int displayRowsForChildren = totalDisplayRows;
		// 2. ������ ������ - ����� � ������ ���������
		int currItemRow = 1; // ����� �� ����� ����� ���� ���� (������� � 1)
		const byte* rc = rItm->Children; // ����� �� ������� �������� ���������
		id = pgm_read_byte(rc); // �� ������ ������� ID ������ ���� (����� � PROGMEM)
		while (id)
		{
			// ������ ������ ����
			if (totalDisplayRows > 0 && pos <= currItemRow ) // ������, ���� �� ����� �� ����� ������� � ���� � ����� ������ ������
			{
				DrawMenuLine(GetMenuById(id), (id == menuByCursor && !hasMarked), rowNum++); // ������������ ������� ������, ���� �� ���� ���������� ����� back
				totalDisplayRows--;
			}
			currItemRow++;
			id = pgm_read_byte(++rc);
		}

		// TODO: ������ ������ (���� ���������� ������� ������� � �� ������ � �����) - ��� ������ back
		if (displayRowsForChildren < totalChildCount)
		{
			// ����� ����� - totalChildCount
			// ������� ��������� - cursorNum (������� � 1)

			//zeos->display.SingleBar(82, 47, 39, 2, PIXEL_ON);
			//zeos->display.Line(83, 17, 83, 20, PIXEL_OFF);
		}
		

	}
	//else
	//{
	//LcdGotoXYFont(0, 3);
	//LcdFStr(FONT_1X, noTitle);
	//}
	
	// ����� � ������
	zeos->display.Update();
}


void MenuManager::DrawMenuTitle(volatile MenuItem* itmRef) const
{
	// ����� ��������
	zeos->display.Goto(0, 0);
	zeos->display.FStr(itmRef->Title, false, false);
	// TODO: �����������
}
void MenuManager::DrawMenuBack(bool isSelected) const
{
	zeos->display.Goto(0, 1);
	// �������, �������� ����
	zeos->display.FStr(isSelected ? currentMenuMark : currentMenuNotMark);
	// ����� ��������
	zeos->display.FStr(menuBackTitle, false, false);
}
byte MenuManager::DrawMenuLine(const MenuItem* itmRef, bool isSelected, unsigned int rowNum) const
{
	zeos->display.Goto(0,rowNum);
	// �������, �������� ����
	zeos->display.FStr(isSelected ? currentMenuMark : currentMenuNotMark);
	// ����� ��������
	return zeos->display.FStr(FONT_1X, itmRef->Title, false, false);
}
