
#include "ofApp.h"

//========================================================================
int main(int argc, char *argv[]){



	vector<string> arguments = vector<string>(argv, argv + argc);
	bool findkinect = false;
	for (const auto& arg : arguments) {
		if (arg.find("kinect")) {
			findkinect = true;
		}
	}
	// let command line get processed before we allocated to avoid depend. issues

	Software2552::init(); //bugbug maybe pass opengl and other parmeters here
	ofApp *app = new ofApp(); // assume ram here or just blow up any way
	app->seekKinect = findkinect; // command line
							
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(app);

}
