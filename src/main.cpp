
#include "ofApp.h"

//========================================================================
int main(int argc, char *argv[]){

	ofApp *app = new ofApp(); // assume ram here or just blow up any way
	app->config.setup();

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(app);

}
