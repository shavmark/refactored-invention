
#include "ofApp.h"

//========================================================================
int main(int argc, char *argv[]){

	ofApp *app = new ofApp(); // assume ram here or just blow up any way

	vector<string> arguments = vector<string>(argv, argv + argc);
	for (const auto& arg : arguments) {
		if (arg.find("kinect")) {
			app->seekKinect = true; // command line
		}
	}

	Software2552::init(); //bugbug maybe pass opengl and other parmeters here

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(app);

}
