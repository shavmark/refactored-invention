#pragma once
#include "ofApp.h"
#include "control.h"
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
		float x =0;
		float y = 0;
		float x2 = 0;
		float y2 = 0;
		float x3 = 0;
		float y3 = 0;
		float z3 = 0;
		Software2552::TCPReader reader;
		ChannelList playlist;
		ColorList colorlist;
		Stage stage;
		ReadOsc read;
		WriteOsc write;
	};


}