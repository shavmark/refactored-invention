
#include "ofApp.h"

//========================================================================
int main(int argc, char *argv[]){
	SystemConfiguration system;
	system.setup();

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunMainLoop();
}
