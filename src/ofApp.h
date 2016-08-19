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
#include "ofxOsc.h"
#include "ofxNetwork.h"
#include "ofxcv.h"
#include "ofxJSON.h"
#include "consts.h"
#include "animation.h"
#include "color.h"
#include "model.h"
#include "shaders.h"
#include "networking.h"
#include "sound.h"
#include "ofxBeat.h"


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
	int getseekArduino() { return seekArduino; }
	int getseekSound() { return seekSound; }
	int getDrawMusic() { return drawMusic; }
	int getSoundGenerator(){ return generateSound; }
	string&getName() { return machineName; }
	static void getName(string&name, shared_ptr<ofxOscMessage>);
	int getWindowNumber() {	return windowNumber	;};
	static int getWindowNumber(shared_ptr<ofxOscMessage>);
	void setFrameRateinOF() { ofSetFrameRate(frameRate); }
	enum Location { left, right, middle, back };
	//bugbug wrap as needed when ready
	vector <string> jsonFile;
	shared_ptr<SystemConfiguration::Window> parent;
	bool installed = false;
	vector <shared_ptr<ofSoundPlayer>>sounds;
	shared_ptr<Software2552::SoundOut> soundout = nullptr; // set, replace etc as needed
private:
	int windowNumber;
	Location location;
	string os;
	int performance = 0;
	string build;
	bool seekKinect = false;
	bool seekArduino = false;
	bool seekSound = false;
	bool generateSound = false;
	bool drawMusic = false;
	int frameRate = 30;
	int windowCount = 1;
	string machineName;
};

#include "timeline.h"
#include "control.h"



// one instance of network items
class OneGlobalInstance {
public:
	string kinectServerIP; // each window could connect to an other kinect bugbug future phase
	shared_ptr<Software2552::PixelsClient>tcpIRIndex = nullptr;
	shared_ptr<Software2552::PixelsClient>tcpBodyIndex = nullptr;
	shared_ptr<Software2552::TCPKinectClient>tcpKinectClient = nullptr;
	shared_ptr<Software2552::ReadOsc>oscClient = nullptr; // everyone can talk to everyone
	shared_ptr<Software2552::Sender>router = nullptr; // everyone can talk to everyone
	shared_ptr<ofArduino>arduino = nullptr;

#ifdef _WIN64
										// every 64 bit windows with a 3.0 usb can talk to everyone else
	shared_ptr<Software2552::KinectBody> kinectBody = nullptr;
	shared_ptr<Software2552::KinectDevice> kinectDevice = nullptr;
#else
	class KinectDevice {
	public:
		string getId() { return ""; }// no kinect in 32 bit worlds
	};
	shared_ptr<KinectDevice> kinectDevice = nullptr;
#endif

};
extern OneGlobalInstance globalinstance;

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

		Software2552::Timeline timeline;

		void audioOut(ofSoundBuffer &outBuffer);
		void audioReceived(float*, int, int);

		AppConfiguration appconfig; // put all app instance here 
		ofxBeat beat; //bugbug not sure where to put this yet, need to support > 1 mic
		float getBand(int i) { return beat.getBand(i); }


		ofVideoGrabber movie;


private:
};

#define BAND(i) ((ofApp*)ofGetAppPtr())->getBand(i)
#define KICK ((ofApp*)ofGetAppPtr())->beat.kick()
#define SNARE ((ofApp*)ofGetAppPtr())->beat.snare()
#define HIHAT ((ofApp*)ofGetAppPtr())->beat.hihat()
#define BEAT ((ofApp*)ofGetAppPtr())->beat
#define MAG (((ofApp*)ofGetAppPtr())->beat.getMagnitude())


