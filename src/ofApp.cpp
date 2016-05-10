#include "ofApp.h"
#include "ofxXmlSettings.h"

void AppConfiguration::setup() {

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
		ofGLWindowSettings settings;
		performance = xmlsettings.getValue("settings:Performance", 0.0); // 0 is slowest, 11 is fastest
		build = __DATE__;//bugbug this is our build version right now, need to add more info bugbug or pass in a data structure
		seekKinect = xmlsettings.getValue("settings:Kinect", false);
		frameRate = xmlsettings.getValue("settings:Framerate", 30);
		machineName = xmlsettings.getValue("settings:MachineName", "bob");
		string loc = xmlsettings.getValue("settings:Location", "none");
		if (loc == "left") {
			location = left;
		}
		else if (loc == "right") {
			location = right;
		}
		else if (loc == "middle") {
			location = middle;
		}
		else if (loc == "back") {
			location = back;
		}
		xmlsettings.pushTag("settings");
		xmlsettings.pushTag("Windows");
		int windowCount = xmlsettings.getNumTags("window");
		for (int i = 0; i < windowCount; i++) {
			xmlsettings.pushTag("window", i);
			shared_ptr<Window> window = std::make_shared<Window>();
			if (!window) {
				return; // too early for this, time to go
			}
			window->x = xmlsettings.getValue("x", 1); //bugbug set proper defaults once working
			window->y = xmlsettings.getValue("y", 1);
			window->width = xmlsettings.getValue("ScreenWidth", 1);  //bugbug maybe -1 means max size or such
			window->height = xmlsettings.getValue("ScreenHeight", 1);
			int jsonCount = xmlsettings.getNumTags("json");
			for (int i = 0; i < jsonCount; i++) {
				window->jsonFile.push_back(xmlsettings.getValue("json", "baddata", i));
			}
			windows.push_back(window);
			xmlsettings.popTag();
		}
		xmlsettings.popTag();
		xmlsettings.popTag();

		settings.width = 800;
		settings.height = 800;
		settings.setPosition(ofVec2f(300, 0));
		// come back to this for multiple windows/ monitors http://blog.openframeworks.cc/post/133404337264/openframeworks-090-multi-window-and-ofmainloop
		settings.setGLVersion(xmlsettings.getValue("settings:OpenglMajor", 4), xmlsettings.getValue("settings:OpenglMinor", 0));
		
		settings.windowMode = ofWindowMode::OF_WINDOW;
		// this kicks off the running of my app    
		ofCreateWindow(settings);
		string s = "ver " + ofToString(settings.glVersionMajor) + "." + ofToString(settings.glVersionMinor);
		ofLog(OF_LOG_NOTICE, s);

}

void AppConfiguration::getName(string &name, shared_ptr<ofxOscMessage>p) {
	if (p) {
		name = p->getArgAsString(2);
	}
}
AppConfiguration::AppConfiguration(shared_ptr<ofxOscMessage>p) {
	if (p) {
		os = p->getArgAsString(0);
		build = p->getArgAsString(1);
		machineName = p->getArgAsString(2);
		location = (Location)p->getArgAsInt(3);
		performance = p->getArgAsInt(4);
		monitorCount = p->getArgAsInt(5); // bugbug likely need to sign on by monitor vs machine
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
		signon->addIntArg(monitorCount); // bugbug likely need to sign on by monitor vs machine
																		 // our app becomes a collection of monitors, devices, speakers all acting as one including Kinect
	}
	return signon;
}

//--------------------------------------------------------------
void ofApp::setup(){

	ofSetWindowTitle("Story Teller");
	//Software2552::SoundIn::setup();// move to timeline or scene
	//Software2552::SoundOut::setup();// move to timeline or scene

	ofSetLogLevel(OF_LOG_NOTICE);//OF_LOG_VERBOSE  OF_LOG_NOTICE
	timeline.setup();//logErrorString
	timeline.readScript("json3.json");
	timeline.start();

	return;
}


//--------------------------------------------------------------
void ofApp::update(){
	timeline.update();
	Software2552::SoundOut::update();// move to timeline or scene
	Software2552::SoundIn::update();// move to timeline or scene
	return;
	
	return;
}
void ofApp::audioIn(float * input, int bufferSize, int nChannels) {
	Software2552::SoundIn::audioIn(input, bufferSize, nChannels);
}
//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer &outBuffer) {
	Software2552::SoundOut::audioOut(outBuffer);
}

//--------------------------------------------------------------
void ofApp::draw(){
	timeline.draw();
	//Software2552::SoundOut::draw();//bugbug move to timeline
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

