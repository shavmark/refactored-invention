#include "ofApp.h"
#include "ofxXmlSettings.h"


OneGlobalInstance globalinstance;

void SystemConfiguration::setup() {
		ofxXmlSettings xmlsettings; // local settings like box performance and type
		xmlsettings.loadFile("settings.xml");

		// performance calc, run compubenchcl and compare it to my dev box which is pretty average so lets say its a 5 of 10 which is generous but
		// its my dev box so I need to be kind, 11 is reserved.
		// my dev box results: face 15Mpixels, TVL1 3.9, ocean 282, particle 128, t rex	1.2 frames/second, video 25, bitcoin 73
		// example a low end box (Intel HD graphics) I have gets: face 4.6, tv .86, ocean 75, partical 57, t rex .49, video 4, bit coin 7.3, this is
		// pretty bad so its about a 2. 
		// based on Compubench (5-9-16) the fastest test yields: face 253, tv 45, ocean, 3099, particle 2283, t rex 16, video 179, bitcoin 918,
		// this is a 10.

		//bugbug each screen gets one of these functions so XML is rooted with video name

		// 32 bit boxes use gfxbench as it supports 32 bit

		// data that is true for all windows
		int performance = xmlsettings.getValue("settings:Performance", 0.0); // 0 is slowest, 11 is fastest
		string build = __DATE__;//bugbug this is our build version right now, need to add more info bugbug or pass in a data structure
		bool seekKinect = xmlsettings.getValue("settings:Kinect", false);
		string machineName = xmlsettings.getValue("settings:MachineName", "bob");
		int major = xmlsettings.getValue("settings:OpenglMajor", 4);// could make each window set this too
		int minor = xmlsettings.getValue("settings:OpenglMinor", 0);
		// per window items

		xmlsettings.pushTag("settings");
		xmlsettings.pushTag("Windows");
		int windowCount = xmlsettings.getNumTags("window");
		for (int i = 0; i < windowCount; i++) {
			xmlsettings.pushTag("window", i);
			shared_ptr<Window> window = std::make_shared<Window>();
			if (!window) {
				return; // too early for this, time to go
			}
			window->app = make_shared<ofApp>();
			if (!window->app) {
				return; 
			}
			ofGLWindowSettings settings;
			window->x = xmlsettings.getValue("x", 1); //bugbug set proper defaults once working
			window->y = xmlsettings.getValue("y", 1);
			window->width = xmlsettings.getValue("ScreenWidth", 1);  //bugbug maybe -1 means max size or such
			window->height = xmlsettings.getValue("ScreenHeight", 1);
			int jsonCount = xmlsettings.getNumTags("json");
			for (int i = 0; i < jsonCount; i++) {
				window->app->appconfig.jsonFile.push_back(xmlsettings.getValue("json", "baddata", i));
			}

			settings.width = xmlsettings.getValue("ScreenWidth", 1);
			settings.height = xmlsettings.getValue("ScreenHeight", 1);
			settings.setPosition(ofVec2f(xmlsettings.getValue("x", 1), xmlsettings.getValue("y", 1)));
			// come back to this for multiple windows/ monitors http://blog.openframeworks.cc/post/133404337264/openframeworks-090-multi-window-and-ofmainloop
			settings.setGLVersion(major, minor);

			settings.windowMode = ofWindowMode::OF_WINDOW;
			// this kicks off the running of my app    
			shared_ptr<ofAppBaseWindow> win = ofCreateWindow(settings);
			window->app->appconfig.windowNumber = i; // just so we know whats coming from where
			window->app->appconfig.performance = performance;
			window->app->appconfig.build = build;
			window->app->appconfig.seekKinect = seekKinect;// only want one per window
			window->app->appconfig.machineName = machineName;
			window->app->appconfig.windowCount = windowCount; // track as an fyi
			window->app->appconfig.frameRate = xmlsettings.getValue("framerate", 30);
			string loc = xmlsettings.getValue("location", "left");
			if (loc == "left") {
				window->app->appconfig.location = AppConfiguration::left;
			}
			else if (loc == "right") {
				window->app->appconfig.location = AppConfiguration::right;
			}
			else if (loc == "middle") {
				window->app->appconfig.location = AppConfiguration::middle;
			}
			else if (loc == "back") {
				window->app->appconfig.location = AppConfiguration::back;
			}
			window->app->appconfig.parent = window; // tie ack
			ofRunApp(win, window->app);

			string s = "ver " + ofToString(settings.glVersionMajor) + "." + ofToString(settings.glVersionMinor);
			ofLog(OF_LOG_NOTICE, s);

			windows.push_back(window);
			xmlsettings.popTag();
		}
		xmlsettings.popTag();
		xmlsettings.popTag();

}

void AppConfiguration::getName(string &name, shared_ptr<ofxOscMessage>p) {
	if (p) {
		name = p->getArgAsString(2);
	}
}
int AppConfiguration::getWindowNumber(shared_ptr<ofxOscMessage>p) {
	if (p) {
		return p->getArgAsInt(6); 
	}
	return -1;// bad news
}

AppConfiguration::AppConfiguration(shared_ptr<ofxOscMessage>p) {
	if (p) {
		os = p->getArgAsString(0);
		build = p->getArgAsString(1);
		machineName = p->getArgAsString(2);
		location = (Location)p->getArgAsInt(3);
		performance = p->getArgAsInt(4);
		windowCount = p->getArgAsInt(5); // bugbug likely need to sign on by monitor vs machine
		windowNumber = p->getArgAsInt(6);
	}

}

shared_ptr<ofxOscMessage>  AppConfiguration::getsignon() {
	shared_ptr<ofxOscMessage> signon = std::make_shared<ofxOscMessage>();
	if (signon) {
#ifdef _WIN32
		// for 64 and 32 bit windows, os ver not tracked
		os = "windows32";
#endif
#ifdef _WIN64
		os = "windows64";
#endif
		// all devices sign in
		signon->addStringArg(os);
		signon->addStringArg(build);
		signon->addStringArg(machineName);
		signon->addIntArg(location);
		signon->addIntArg(performance);
		signon->addIntArg(windowCount); 
		signon->addIntArg(windowNumber);
	 // our app becomes a collection of windows, input devices, speakers all acting as one including Kinect.  A machine can drive any number of monitors
	}
	return signon;
}


//--------------------------------------------------------------
void ofApp::setup(){

	ofSetLogLevel(OF_LOG_NOTICE);//OF_LOG_VERBOSE  OF_LOG_NOTICE

	ofSetWindowTitle("Story Teller");
	
	Software2552::SoundOut::setup();

	timeline.setup();//logErrorString
	timeline.readScript("json3.json");
	timeline.start();

	return;
}
void ofApp::audioOut(ofSoundBuffer &outBuffer) {
	Software2552::SoundOut::audioOut(outBuffer);
}
//--------------------------------------------------------------
void ofApp::update(){
	Software2552::SoundOut::update();
	timeline.update();
	return;
}

//--------------------------------------------------------------
void ofApp::draw(){
	Software2552::SoundOut::draw();
	timeline.draw();
	return;


#ifndef TARGET_PROGRAMMABLE_GL    
	glShadeModel(GL_SMOOTH); //some model / light stuff
#endif
}

void ofApp::mouseDragged(int x, int y, int button) {
}

void ofApp::exit() {
	
}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}
//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::keyPressed(int key) {
}

