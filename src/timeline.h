#pragma once
#include "ofApp.h"
#include "control.h"

#ifdef _WIN64
// kinect only works on64 bit windows
#include "kinect2552.h"
#endif

// join data and view, this class knows about both the data and how its shown

namespace Software2552 {
	class Timeline {
	public:
		Timeline();
		void start();
		void setup();
		void update();
		void draw();
		void pause();
		void resume();
		const int frameRate = 60;

		bool readScript(const string& path);

	private:
		//bugbug likely to go into ofApp.cpp once things settle down and timeline just goes away
		float x =0;
		float y = 0;
		float x2 = 0;
		float y2 = 0;
		float x3 = 0;
		float y3 = 0;
		float z3 = 0;
		bool clientInstalled = false;
		ChannelList playlist;
		ColorList colorlist;
		shared_ptr<Stage> stage = nullptr;
		shared_ptr<Client>client = nullptr; // everyone can talk to everyone
		shared_ptr<Sender>router = nullptr; // everyone can talk to everyone
#ifdef _WIN64
		// every 64 bit windows with a 3.0 usb can talk to everyone else
		shared_ptr<KinectDevice> kinectDevice = nullptr;
		shared_ptr<KinectBody> kinectBody = nullptr;
#endif
	};


}