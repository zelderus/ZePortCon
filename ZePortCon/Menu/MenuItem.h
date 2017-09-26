/*
 * MenuItem.h
 *
 * Created: 13.09.2017 9:42:58
 *  Author: kotenko_kg
 */ 


#ifndef MENUITEM_H_
#define MENUITEM_H_

#include "../Types/defines.h"



//#define MENU_TITLE_SIZE 20




class MenuItem
{
public:
	const byte* Title;
	int ParentId;			// номер меню родителя, на который надо вернуться
	const byte* Children;	// указатель на строку, в которой зашифрованы ID менюшек детей (по байту на один ID)
	unsigned int AppId;		// ссылка на функцию для выполнения (если нет детей)

	unsigned int LastCursorNum;

	MenuItem();
	MenuItem(const byte* title, int parentId, const byte* children, const unsigned int appId = 0);

	void DoTest();
};




#endif /* MENUITEM_H_ */