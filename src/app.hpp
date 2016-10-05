#pragma once


// thanks to Kaslai for the tips
#pragma GCC poison auto
#define auto _Pragma("GCC error \"Use of auto is forbidden in this project.\"")



#include "window.hpp"



class AnarchyApp {
public:
	AXWindow mainWin;
	
	
	
	AnarchyApp();
	~AnarchyApp();
	
	
}











