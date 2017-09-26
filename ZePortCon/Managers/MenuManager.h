/*
 * MenuManager.h
 *
 * Created: 13.09.2017 11:46:21
 *  Author: kotenko_kg
 */ 


#ifndef MENUMANAGER_H_
#define MENUMANAGER_H_


#include <avr/pgmspace.h>
#include "../Types/defines.h"
#include "../Menu/MenuItem.h"
#include "../zeos.h"



const byte currentMenuMark[] PROGMEM = { 0x10, 0x00 }; //">";
const byte currentMenuNotMark[] PROGMEM = " ";
const byte menuBackTitle[] PROGMEM = "..";



typedef enum
{
	MENU_DRAW	= 1,	// ��������� ����
	APP_RUN		= 2		// ������ ����������
} MenuManagerResponse;




class MenuManager
{
private:
	Zeos* zeos;
	
	unsigned int id;
	unsigned int _itmId;
	volatile MenuItem* _itmRef;
	unsigned int _itmCursorId;
	bool _isCursorBack;
	unsigned int _cursorNum;
	MenuItem _items[12];


public:
	byte debugByte;

	void Init(Zeos* zeos);
	void SetMenu(byte menuId);
	MenuManagerResponse SetParentMenu();	// ������� ������� ������� ���� �����������, ���� ��� ����
	MenuManagerResponse RunCurrentMenu();	// ������ ����, ���� ������� ��� ������ �����
	const MenuItem* GetMenuById(byte menuId) const;
	volatile MenuItem* GetCurrentMenu() const;	// ������� ����

	bool IsCursorBack() const;
	byte GetMenuCursorId() const;					// ID ����, �� ������� ������
	const MenuItem* GetMenuByCursor();	// ����, �� ������� ������ (��������� � ������������ - �������)
	MenuManagerResponse SetCursorToPrev();
	MenuManagerResponse SetCursorToNext();

	// draws
	void DrawCurrentMenu() const;
	

private:
	
	void SetCursorToMenu(byte menuId, unsigned int cursorNum);
	void SetCursorToChildrenByNum(byte menuId, unsigned int num = 1);	// ������������� ������ �� ���� ������� �� ������ �������
	unsigned int GetMenuIdFromChildrenByNum(byte menuId, unsigned int num = 1);	// ID �� ���� ������� �� ������ �������
	
	// draws
	void DrawMenuTitle(volatile MenuItem* itmRef) const;
	void DrawMenuBack(bool isSelected) const;
	byte DrawMenuLine(const MenuItem* itmRef, bool isSelected, unsigned int rowNum) const;
	
};


#endif /* MENUMANAGER_H_ */