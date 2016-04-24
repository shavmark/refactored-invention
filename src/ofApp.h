#pragma once
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
#include "animation.h"
#include "color.h"
#include "model.h"
#include "scenes.h"
#include "networking.h"
#include "timeline.h"


#define STRINGIFY(p) #p
namespace Software2552 {
	void init();

	// see about this, add things like Trace() << "stuff" to be more like OF
	class Trace {
	public:

		static void logError2(const string& error, char*file, int line);
		static void logVerbose2(const string& message, char*file, int line) {
			if (ofGetLogLevel() >= OF_LOG_VERBOSE) {
				ofLog(OF_LOG_VERBOSE, buildString(message, file, line));
			}
		}
		static void logTraceBasic(const string& message, char *name);
		static void logTraceBasic(const string& message);
		static void logTrace2(const string& message, char*file, int line);
		static void logError2(HRESULT hResult, const string&  message, char*file, int line);
		static string buildString(const string& errorIn, char* file, int line);
		static std::string wstrtostr(const std::wstring &wstr);

		// get the right line number bugbug add DEBUG ONLY
#define logError(p1, p2) Trace::logError2(p1, p2, __FILE__, __LINE__)
#define logErrorString(p1) Trace::logError2(p1, __FILE__, __LINE__)
#if _DEBUG
#define logVerbose(p1) Trace::logVerbose2(p1, __FILE__, __LINE__)
#else
#define logVerbose(p1) 
#endif
#define logTrace(p1) Trace::logTrace2(p1, __FILE__, __LINE__)
	};
}

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

		void ofApp::drawScene(bool isPreview){
	
	ofEnableDepthTest();

	if (false) { // keep to learn how to tie to kinect
		ofPushStyle();
		ofSetColor(150, 100, 100);
		ofDrawGrid(1.0f, 5.0f, true);
		
		ofSetColor(255);
		
		//--
		//draw camera preview
		//
		headTrackedCamera.transformGL();
		
		ofPushMatrix();
		ofScale(0.002f, 0.002f, 0.002f);
		ofNode().draw();
		ofPopMatrix();
		
		ofMultMatrix(headTrackedCamera.getProjectionMatrix().getInverse());
		
		ofNoFill();
		ofDrawBox(2.0f);
		
		headTrackedCamera.restoreTransformGL();
		//
		//--
		
		//--
		//draw window preview
		//
		window.clear();
		window.addVertex(windowTopLeft);
		window.addVertex(windowBottomLeft);
		window.addVertex(windowBottomRight);
		window.setMode(OF_PRIMITIVE_LINE_STRIP);
		window.draw();
		glPointSize(3.0f);
		window.drawVertices();
		//
		//--
        ofPopStyle();
	}
	
	ofPushStyle();
	ofNoFill();
	ofColor col(200,100,100);
	for (float z = 0.0f; z > -40.0f; z-= 0.1f){
		col.setHue(int(-z * 100.0f + ofGetElapsedTimef() * 10.0f) % 360);
		ofSetColor(col);
		ofDrawRectangle(-windowWidth / 2.0f, -windowHeight / 2.0f, z, windowWidth, windowHeight);
	}
	ofPopStyle();
	
	ofPushStyle();
	ofEnableSmoothing();
	ofSetColor(255);
	ofSetLineWidth(5.0f);
	ofBeginShape();
	for (unsigned int i=0; i<headPositionHistory.size(); i++) {
		ofPoint vertex(headPositionHistory[i].x, headPositionHistory[i].y, -float( headPositionHistory.size() - i ) * 0.05f);
		ofCurveVertex(vertex);
	}
	ofEndShape(false);
	ofPopStyle();
	
	ofDisableDepthTest();
}

		ofLight	light;
		ofEasyCam camera;
		ofVideoPlayer test;
		ofMaterial material;
		int   appFPS;
		float sequenceFPS;
		bool  bFrameIndependent;
		
		void circleResolutionChanged(int & circleResolution);
		void ringButtonPressed();
		bool bHide;
		
		int years;

		ofSoundPlayer ring;
		ofImage backgroundImage;
		// example only, but a good one, SceneAnimator sa;
		
		ofPlanePrimitive plane;
		ofCamera cam;
		float angle;
		ofVec3f previous, current;
		ofEasyCam easyCam;

		deque<ofVec3f> pathVertices;
		ofMesh pathLines;
		ofCamera headTrackedCamera;
		float windowWidth;
		float windowHeight;
		ofVec3f windowTopLeft;
		ofVec3f windowBottomLeft;
		ofVec3f windowBottomRight;
		float viewerDistance;

		deque<ofPoint> headPositionHistory;

		ofVboMesh window;
};
