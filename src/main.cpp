#include <algorithm>
#include <functional> 
#include <cctype>
#include <locale>

#include <sapi.h>  
#include <sphelper.h>  
#include <Synchapi.h>
#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	Software2552::init();

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
