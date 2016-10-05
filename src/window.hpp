#pragma once

#include "../libs/glm/glm.hpp"


using namespace glm;







class AXWindow {
public:
	Display*     display;
	Window       rootWin;
	Window       clientWin;
	GLXContext   glctx;
	
	XVisualInfo* vi;
	Colormap     colorMap;
	
	XWindowAttributes winAttr;
	
	void (*onExpose)(AXWindow*, void*);
	void* onExposeData;

	int targetMSAA;
	std::string windowTitle;
	
	
	Bool ready;
	
	AXWindow();
	
	int init();
	void processEvents(InputState* st, int max_events)
}





XErrorEvent* xLastError;
char xLastErrorStr[1024];




