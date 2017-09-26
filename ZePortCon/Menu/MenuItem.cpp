/*
 * MenuItem.cpp
 *
 * Created: 13.09.2017 9:42:48
 *  Author: kotenko_kg
 */ 



 #include "MenuItem.h"


 MenuItem::MenuItem()
 {
	this->ParentId = 0;
	this->AppId = 0;
	this->LastCursorNum = 1;
 }
 MenuItem::MenuItem(const byte* title, int parentId, const byte* children, const unsigned int appId)
 {
	this->LastCursorNum = 1;
	this->Title = title;
	this->ParentId = parentId;
	this->Children = children;
	this->AppId = appId;
 }

 void MenuItem::DoTest()
 {
	

 }