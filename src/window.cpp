
#include <string>
#include <limits.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "../libs/glm/glm.hpp"
#include "window.hpp"


void initGLEW();
 
 
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;


const int context_attr[] = {
	GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
	GLX_CONTEXT_MINOR_VERSION_ARB, 3,
#ifdef USE_KHR_DEBUG
	GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
	None
};


const int visual_attr[] = {
	GLX_X_RENDERABLE    , True,
	GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
	GLX_RENDER_TYPE     , GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
	GLX_RED_SIZE        , 8,
	GLX_GREEN_SIZE      , 8,
	GLX_BLUE_SIZE       , 8,
	GLX_ALPHA_SIZE      , 8,
	GLX_DEPTH_SIZE      , 24,
	GLX_STENCIL_SIZE    , 8,
	GLX_DOUBLEBUFFER    , True,
	//GLX_SAMPLE_BUFFERS  , 1, // antialiasing
	//GLX_SAMPLES         , 4,
	None
};
	


XErrorEvent* xLastError = NULL;
char xLastErrorStr[1024] = {0};

int xErrorHandler(Display *dpy, XErrorEvent *ev) {
	
	xLastError = ev;
	XGetErrorText(dpy, ev->error_code, xLastErrorStr, 1024);
	
	fprintf(stderr, "X Error %d: %s", ev->error_code, xLastErrorStr);
	
    return 0;
}



 
// this function will exit() on fatal errors. what good is error handling then?
int AXWindow::init() {
	
	GLXFBConfig* fbconfigs;
	GLXFBConfig chosenFBC;
	int fbcount, i;
	int best_fbc = -1, best_num_samp = -1;
	XSetWindowAttributes setWinAttr;
	
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	
	
	display = XOpenDisplay(NULL);
	if(display == NULL) {
		printf("Cannot connect to X server\n");
		exit(1);
	}

	rootWin = DefaultRootWindow(display);
	
	
	// find a framebuffer config
	fbconfigs = glXChooseFBConfig(display, DefaultScreen(display), visual_attr, &fbcount);
	if(!fbconfigs) {
		fprintf(stderr, "No usable framebuffer config\n" );
		exit(1);
	}
	
	
	// try to get the requested MSAA, or the closest to it without going over
	for(i = 0; i < fbcount; i++) {
		XVisualInfo* vi;
		int samp_buf, samples;
		
		vi = glXGetVisualFromFBConfig(display, fbconfigs[i]);
		if(!vi) continue;
			
		glXGetFBConfigAttrib(display, fbconfigs[i], GLX_SAMPLE_BUFFERS, &samp_buf);
		glXGetFBConfigAttrib(display, fbconfigs[i], GLX_SAMPLES, &samples);
		glerr("samples");
		
		if(best_fbc < 0 || samp_buf && samples > best_num_samp && samples <= targetMSAA) {
			best_fbc = i;
			best_num_samp = samples;
		}
		
		XFree(vi);
	}
	
	chosenFBC = fbconfigs[best_fbc];
	
	XFree(fbconfigs);
	
	
	// Get a visual
	vi = glXGetVisualFromFBConfig(display, chosenFBC);
	
	
	colorMap = XCreateColormap(display, rootWin, vi->visual, AllocNone);
	setWinAttr.colormap = colorMap;
	setWinAttr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

	clientWin = XCreateWindow(display, rootWin, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &setWinAttr);

	XMapWindow(display, clientWin);
	
	XStoreName(display, clientWin, windowTitle);
	
	// don't check for supported extensions, just fail hard and fast if the computer is a piece of shit.
	
	// cause putting now-ubiquitous stuff in headers is lame...
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
	if(!glXCreateContextAttribsARB) {
		fprintf(stderr, "glXCreateContextAttribsARB() not found. Upgrade your computer.\n" );
		exit(1);
	}
	
	XSetErrorHandler(&xErrorHandler);
	
	glctx = glXCreateContextAttribsARB(display, chosenFBC, 0, True, context_attr);
	
	
	// squeeze out any errors
	XSync(display, False);
	
	
	glXMakeCurrent(display, clientWin, glctx);
	
	
	// have to have a current GLX context before initializing GLEW
	initGLEW();
	
#ifdef USE_KHR_DEBUG
	initKHRDebug();
#endif
	
}




#define CLAMP(min, mid, max) MIN(max, MAX(mid, min))

AXWindow::AXWindow() {
	
	
}

AXWindow::~AXWindow() {
	
	
}

void AXWindow::processEvents(InputState* st, int max_events) {
	
	
	XEvent xev;
	int evcnt;
	int x, y;
	float fx, fy;

	int rootX, rootY, clientX, clientY;
	Window rootReturn, clientReturn;
	unsigned int mouseMask;

	
	if(max_events <= 0) max_events = INT_MAX;
	
	// bottleneck :)
	clearInputState(st);
	
	if(XQueryPointer(display, clientWin, &rootReturn, &clientReturn, &rootX, &rootY, &clientX, &clientY, &mouseMask)) {
		if(winAttr.height > 0 && winAttr.width > 0) {
			st->cursorPosPixels.x = CLAMP(0, clientX, winAttr.width);
			st->cursorPosPixels.y = CLAMP(0, winAttr.height - clientY, winAttr.height);
		
			st->cursorPos.x = clientX / winAttr.width;
			st->cursorPos.y = 1.0 - (clientY / winAttr.height); // opengl is inverted to X
		}
	}
	
	for(evcnt = 0; XPending(display) && evcnt < max_events; evcnt++) {
		XNextEvent(display, &xev);
		
		
		
		// capture expose events cause they're useful. fullscreen games are for wimps who can't ultratask.
		if(xev.type == Expose) {
			// update some standard numbers
			// there's currently risk of a race condition if anything tries to read winAttr before the expose event fires
			XGetWindowAttributes(display, clientWin, &winAttr);
			
			glViewport(0, 0, winAttr.width, winAttr.height);
			
			if(onExpose)
				(*onExpose)(xs, onExposeData);
			
			ready = 1;
			
		}
		
		if(xev.type == KeyPress) {
			KeySym s;
			
			int keycode = ((XKeyEvent*)&xev)->keycode;
			//s = XLookupKeysym((XKeyEvent*)&xev, 0);
			
			//printf("key: %d %c\n", keycode, keycode);
			
			st->keyState[keycode] |= IS_KEYPRESSED | IS_KEYDOWN;
		}
		if(xev.type == KeyRelease) {
			
			int keycode = ((XKeyEvent*)&xev)->keycode;
			st->keyState[keycode] &= !IS_KEYDOWN;
		}
		
		// mouse events
		if(xev.type == ButtonPress) {
// 			st->clickPos.x = xev.xbutton.x / (float)winAttr.width;
// 			st->clickPos.y = (winAttr.height - xev.xbutton.y) / (float)winAttr.height;
// 			st->clickButton = xev.xbutton.button;
			st->buttonDown = xev.xbutton.button;
		}
		if(xev.type == ButtonRelease) {
			st->clickPos.x = xev.xbutton.x / (float)winAttr.width;
			st->clickPos.y = (winAttr.height - xev.xbutton.y) / (float)winAttr.height;
			st->clickButton = xev.xbutton.button;
			st->buttonUp = xev.xbutton.button;
			
		}
		
		
		
	}
	
	
	
}



void clearInputState(InputState* st) {
	int i;
	
	for(i = 0; i < 256; i++) {
		st->keyState[i] &= IS_KEYDOWN;
	}
	
	st->clickPos.y = -1;
	st->clickPos.x = -1;
	st->clickButton = 0;
	st->buttonUp = 0;
	st->buttonDown = 0;
	
}





void initGLEW() {
	GLenum err;
	
	// this is some global that GLEW declares elsewhere
	glewExperimental = GL_TRUE; // workaround for broken glew code in 1.10 and below (at least) - gl3.2+ contexts fail otherwise
	
	err = glewInit();
	if (GLEW_OK != err) {
		// we're fucked, just bail
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		exit(1);
	}
	
	fprintf(stdout, "Initialized GLEW %s\n", glewGetString(GLEW_VERSION));
	glerr("existing error on glew init");
}