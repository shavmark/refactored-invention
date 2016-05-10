#pragma once
#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include <sapi.h>  
#include <sphelper.h> 
#include <Synchapi.h>
#include <algorithm>
#include <functional> 
#include <cctype>
#include <locale>
// For M_PI and log definitions
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <memory>
#include <map>
// Microsoft Speech Platform SDK 11
#include <sapi.h>
#include <sphelper.h> // SpFindBestToken()
#include <strsafe.h>
#include <intsafe.h>
#include <Synchapi.h>
#include <cstdarg>
#include <forward_list>
#include <unordered_map>
#include "ole2.h"
#include <istream>
#pragma comment( lib, "sapi.lib" )
// keep all MS files above ofmain.h
#include "ofMain.h"
#include "ofxJSON.h"
#include "Poco/Foundation.h"
#include "Poco/DateTime.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeParser.h"
#include "Poco/DateTimeFormatter.h"

#include "ofFileUtils.h"

template<typename T>void setIfGreater(T& f1, T f2) {
	if (f2 > f1) {
		f1 = f2;
	}
}

#include "ofxJSON.h"
#include "consts.h"
#include "animation.h"
#include "color.h"
#include "model.h"
#include "networking.h"
#define STRINGIFY(p) #p
class ofApp;
class SystemConfiguration {
public:
	void setup();
	class Window {
	public:
		int x, y, width, height;
		shared_ptr<ofApp> app; // not sure yet but lets see if it works here
	};
	vector<shared_ptr<Window>>& getWindows() { return windows; }
private:
	vector<shared_ptr<Window>> windows;// pair windows, json
};

class AppConfiguration {
public:
	AppConfiguration() {}
	AppConfiguration(shared_ptr<ofxOscMessage>);
	friend class SystemConfiguration;
	shared_ptr<ofxOscMessage> getsignon();
	int getPerformance() { return performance; }
	int getFramerate() { return frameRate; }
	int getseekKinect() { return seekKinect; }
	string&getName() { return machineName; }
	static void getName(string&name, shared_ptr<ofxOscMessage>);
	int getWindowNumber() {	return windowNumber	;};
	static int getWindowNumber(shared_ptr<ofxOscMessage>);
	void setFrameRateinOF() { ofSetFrameRate(frameRate); }
	enum Location { left, right, middle, back };
	//bugbug wrap as needed when ready
	vector <string> jsonFile;
	shared_ptr<SystemConfiguration::Window> parent;

private:
	int windowNumber;
	Location location;
	string os;
	int performance = 0;
	string build;
	bool seekKinect = false;
	int frameRate = 30;
	int windowCount = 1;
	string machineName;
};

#include "control.h"
#include "scenes.h"
#include "timeline.h"


class ofApp : public ofBaseApp{

	public:
		ofApp():ofBaseApp(){};

		void setup();
		void update();
		void draw();
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void keyPressed(int key);
		void exit();

		// audio
		void audioOut(ofSoundBuffer &outBuffer);
		//in
		void audioIn(float * input, int bufferSize, int nChannels);
		// audio

		Software2552::Timeline timeline;

		void ofApp::drawScene(bool isPreview) {}
		AppConfiguration appconfig;

private:

};


