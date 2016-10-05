
#include <string>

#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "app.hpp"




AnarchyApp app;

InputState input;


int main(int argc, char* argv[]) {
	int first = 1;
	int configStatus = 0;
	
	app.mainWin.targetMSAA = 4;
	app.mainWin.windowTitle = "Anarchy FM";
	
	app.mainWin.init();
	
	while(1) {
		mainWin.processEvents(&input, -1);
		
		if (first && xs.ready) {
			//initGame(&xs, &game);
			first = 0;
		}
		
		if(xs.ready) {
			//gameLoop(&xs, &game, &input);
		}
		
		
		if(game.frameSpan < 1.0/60.0) {
			// shitty estimation based on my machine's heuristics, needs improvement
			float sleeptime = (((1.0/60.0) * 1000000) - (game.frameSpan * 1000000)) * .7;
 			//printf("sleeptime: %f\n", sleeptime / 1000000);
			//sleeptime = 1000;
			if(sleeptime > 0) usleep(sleeptime); // problem... something is wrong in the math
		}
	}
	
	
	
	
	return 0;
}




 
 




 
 
